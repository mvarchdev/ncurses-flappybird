/// @brief Maximum of key and value length
#define CONFIG_ARG_MAX_BYTES 128

/// @brief Struct for storing one key/value data
typedef struct config_option config_option;
/// @brief Type of struct for storing one key/value data
typedef config_option *config_option_t;

/// @brief Struct for storing one key/value data
struct config_option
{
    config_option_t prev;
    char key[CONFIG_ARG_MAX_BYTES];
    char value[CONFIG_ARG_MAX_BYTES];
};

config_option_t read_config_file(char *path);
int get_int_key_value(char *filename, char *key);
int set_int_key_value(char *tmpfile, char *datafile, char *key, int value);