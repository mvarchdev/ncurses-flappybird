// Copyright 2022 <Maros Varchola - mvarchdev>

#ifndef FLAPPYBIRD_CONFPARSER_H
#define FLAPPYBIRD_CONFPARSER_H

/// @brief Maximum key and value length in parsed config files.
#define CONFIG_ARG_MAX_BYTES 128

/// @brief Struct for storing one key/value entry.
typedef struct config_option config_option;
/// @brief Type alias for a linked config entry pointer.
typedef config_option *config_option_t;

/// @brief Struct for storing one key/value entry.
struct config_option {
  config_option_t prev;
  char key[CONFIG_ARG_MAX_BYTES];
  char value[CONFIG_ARG_MAX_BYTES];
};

config_option_t read_config_file(const char *path);
void free_config_options(config_option_t options);
int get_int_key_value(const char *filename, const char *key);
int set_int_key_value(const char *tmpfile, const char *datafile, const char *key, int value);

#endif  // FLAPPYBIRD_CONFPARSER_H
