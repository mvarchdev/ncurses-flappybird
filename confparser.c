// Copyright 2022 <Maros Varchola - mvarchdev>

#include "confparser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/// @brief Read config file
/// @param path Path to config file
/// @return Pointer to allocated config file data
config_option_t read_config_file(char *path) {
  FILE *fp;

  if ((fp = fopen(path, "r+")) == NULL) return NULL;

  config_option_t last_co_addr = NULL;

  while (1) {
    config_option_t co = NULL;
    if ((co = (config_option_t)calloc(1, sizeof(config_option))) == NULL)
      continue;
    memset(co, 0, sizeof(config_option));
    co->prev = last_co_addr;
    // char tmp = 0;
    char lineout[CONFIG_ARG_MAX_BYTES * 2] = {0};
    fgets(lineout, (CONFIG_ARG_MAX_BYTES * 2) - 1, fp);
    char *newlinec = strstr(lineout, "\n");
    if (newlinec) *newlinec = '\0';

    char *endkey = strstr(lineout, "=");
    char *startvalue = NULL;
    if (endkey && endkey - 1 != NULL) {
      startvalue = endkey + 1;
      endkey--;
      while (*(endkey) == ' ')
        if (endkey <= lineout)
          break;
        else
          endkey--;
      *(endkey + 1) = '\0';
    }

    while (startvalue && *(startvalue) == ' ')
      if (startvalue >= lineout + ((2 * CONFIG_ARG_MAX_BYTES) - 2))
        break;
      else
        startvalue++;

    if (!startvalue) startvalue = lineout;

    strcpy(co->key, lineout);
    strcpy(co->value, startvalue);

    if (lineout[0] == '\0') {
      if (feof(fp)) {
        break;
      }
      if (co->key[0] == '#') {
        while (fgetc(fp) != '\n') {
          // Do nothing (to move the cursor to the end of the line).
        }
        free(co);
        continue;
      }

      free(co);
      continue;
    }

    // printf("Key: %s\nValue: %s\n", co->key, co->value);
    last_co_addr = co;
  }

  return last_co_addr;
}

/// @brief Get config file integer value of key
/// @param filename Config name to use
/// @param key Key to find
/// @return Integer value of key, -1 if not found
int get_int_key_value(char filename[], char key[]) {
  config_option_t sfile = read_config_file(filename);
  if (!sfile) return -1;

  if (sfile->key[0] == '\0') return -1;

  while (sfile) {
    if (strcmp(sfile->key, key) == 0) {
      int outlvl = atoi(sfile->value);
      free(sfile);
      return outlvl;
    }
    config_option_t prev = sfile->prev;
    free(sfile);
    sfile = prev;
  }

  return -1;
}

/// @brief Set integer to config file based on key
/// @param tmpfilename Temporary filename
/// @param datafile Config file to use
/// @param key Key to set
/// @param value Vaule to assign to key
/// @return Error code
int set_int_key_value(char tmpfilename[], char datafile[], char key[],
                      int value) {
  if (!tmpfilename || !datafile || !key) return -1;

  if (strcmp("", key) == 0) return -1;

  FILE *tmpfile = fopen(tmpfilename, "w");
  FILE *dfile = fopen(datafile, "a+");

  if (!tmpfile || !dfile) {
    if (dfile) fclose(dfile);
    if (tmpfile) fclose(tmpfile);

    return -1;
  }

  rewind(dfile);

  int bufferLength = 128;
  char line[bufferLength];
  memset(line, '\0', sizeof(char) * bufferLength);

  while (fgets(line, bufferLength, dfile))
    if (strstr(line, key) == NULL) {
      fputs(line, tmpfile);
      if (line[strlen(line) - 1] != '\n') fputc('\n', tmpfile);
    }

  fprintf(tmpfile, "%s = %d\n", key, value);

  fclose(dfile);
  fclose(tmpfile);

  remove(datafile);
  rename(tmpfilename, datafile);

  return 0;
}
