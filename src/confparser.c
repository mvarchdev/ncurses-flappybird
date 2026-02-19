// Copyright 2022 <Maros Varchola - mvarchdev>

#include "flappybird/confparser.h"

#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *ltrim(char *str) {
  while (*str != '\0' && isspace((unsigned char)(*str))) {
    str++;
  }
  return str;
}

static void rtrim(char *str) {
  size_t len = strlen(str);
  while (len > 0 && isspace((unsigned char)str[len - 1])) {
    str[--len] = '\0';
  }
}

static char *trim(char *str) {
  char *start = ltrim(str);
  rtrim(start);
  return start;
}

static void copy_bounded(char *dest, const char *src) {
  snprintf(dest, CONFIG_ARG_MAX_BYTES, "%s", src);
}

static bool line_matches_key(const char *line, const char *key) {
  char local_copy[CONFIG_ARG_MAX_BYTES * 2] = {0};
  snprintf(local_copy, sizeof(local_copy), "%s", line);

  char *content = trim(local_copy);
  if (content[0] == '\0' || content[0] == '#' || content[0] == ';') {
    return false;
  }

  char *separator = strchr(content, '=');
  if (separator == NULL) {
    return false;
  }

  *separator = '\0';
  return strcmp(trim(content), key) == 0;
}

config_option_t read_config_file(const char *path) {
  if (path == NULL) {
    return NULL;
  }

  FILE *fp = fopen(path, "r");
  if (fp == NULL) {
    return NULL;
  }

  config_option_t last_option = NULL;
  char line_buffer[CONFIG_ARG_MAX_BYTES * 2] = {0};

  while (fgets(line_buffer, sizeof(line_buffer), fp) != NULL) {
    char *line = trim(line_buffer);
    if (line[0] == '\0' || line[0] == '#' || line[0] == ';') {
      continue;
    }

    char *separator = strchr(line, '=');
    if (separator == NULL) {
      continue;
    }

    *separator = '\0';
    char *key = trim(line);
    char *value = trim(separator + 1);
    if (key[0] == '\0') {
      continue;
    }

    config_option_t option = calloc(1, sizeof(config_option));
    if (option == NULL) {
      free_config_options(last_option);
      fclose(fp);
      return NULL;
    }

    copy_bounded(option->key, key);
    copy_bounded(option->value, value);
    option->prev = last_option;
    last_option = option;
  }

  fclose(fp);
  return last_option;
}

void free_config_options(config_option_t options) {
  while (options != NULL) {
    config_option_t previous = options->prev;
    free(options);
    options = previous;
  }
}

int get_int_key_value(const char *filename, const char *key) {
  if (filename == NULL || key == NULL || key[0] == '\0') {
    return -1;
  }

  config_option_t options = read_config_file(filename);
  if (options == NULL) {
    return -1;
  }

  int value = -1;
  for (config_option_t cursor = options; cursor != NULL; cursor = cursor->prev) {
    if (strcmp(cursor->key, key) == 0) {
      value = atoi(cursor->value);
      break;
    }
  }

  free_config_options(options);
  return value;
}

int set_int_key_value(const char *tmpfilename, const char *datafile, const char *key, int value) {
  if (tmpfilename == NULL || datafile == NULL || key == NULL || key[0] == '\0') {
    return -1;
  }

  FILE *tmpfile = fopen(tmpfilename, "w");
  if (tmpfile == NULL) {
    return -1;
  }

  FILE *existing_data = fopen(datafile, "r");
  char line[CONFIG_ARG_MAX_BYTES * 2] = {0};

  if (existing_data != NULL) {
    while (fgets(line, sizeof(line), existing_data) != NULL) {
      if (!line_matches_key(line, key)) {
        fputs(line, tmpfile);
        if (line[strlen(line) - 1] != '\n') {
          fputc('\n', tmpfile);
        }
      }
    }
    fclose(existing_data);
  }

  fprintf(tmpfile, "%s = %d\n", key, value);
  fclose(tmpfile);

  remove(datafile);
  if (rename(tmpfilename, datafile) != 0) {
    remove(tmpfilename);
    return -1;
  }

  return 0;
}
