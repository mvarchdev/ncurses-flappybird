// Copyright 2022 <Maros Varchola - mvarchdev>

#include <stdbool.h>
#include <stdint.h>

#include "confparser.h"

/// @brief For everything there will be used meters computation, for render it
/// will be converted to characters - this constant defines how many characters
/// equals one meter
#define METERTOCHARS 0.5

// IN CHARACTER NOTATION
/// @brief Map size x
#define MAPSIZEX 130
/// @brief Map size y
#define MAPSIZEY 28
/// @brief Map margin
#define MAPMARGIN 1
/// @brief Full render area margin
#define OUTERMARGIN 5
/// @brief Render area border width
#define BORDERWIDTH 1
/// @brief Pipe width
#define PIPEWIDTH 3

/// @brief Path to assets folder
#define ASSETS_FOLDER "./assets"
/// @brief Path to settings folder
#define SETTINGS_FOLDER ASSETS_FOLDER "/settings"
/// @brief Path to settings file
#define SETTINGS_FILE SETTINGS_FOLDER "/settings.conf"

/// @brief Menu items count
#define menu_inem_n 6

/// @brief Struct to define one pipe
typedef struct fbpipe {
  /// @brief Pipe width
  int pipewidth;
  /// @brief Actual x position
  int position;
  /// @brief Upper pipe height
  int upheight;
  /// @brief Down pipe height
  int downheight;
  /// @brief Last time moved
  long long last_time_moved;
  /// @brief If the pipe is already used
  bool enabled;
} fbpipe;

/// @brief Struct to define one level
typedef struct level {
  /// @brief Level number
  int levelnumber;
  /// @brief Optional - levelname
  char levelname[50];
  /// @brief Background color (bits)
  int bgcolor;
  /// @brief Color of pipe border fg and bg
  int pipe_color_brd;
  /// @brief Color of pipe body fg and bg
  int pipe_color_body;
  /// @brief Start speed in meaning - meters per second of screen/map
  float start_speed;
  /// @brief Speed increase per minute
  float speed_increase;
  /// @brief Minimum space between pipes in character notation
  int minimum_space;
  /// @brief Maximum space between pipes in character notation
  int maximum_space;
  /// @brief Minimum width of pipes
  int minimum_width;
  /// @brief Maximum width of pipes
  int maximum_width;
  /// @brief Minimum distance of pipes
  int minimum_distance;
  /// @brief Maximum distance of pipes
  int maximum_distance;
  /// @brief Minimum distance of spaces
  int minimum_distance_space;
  /// @brief Maximum distance of spaces
  int maximum_distance_space;
  /// @brief Gravitation multiplier (multiply of gravity constant)
  float gravity_multiply;
  /// @brief Jump upper speed
  float jump_speed;
  /// @brief Max lives of bird
  int max_lives;
  /// @brief If the level is correctly loaded
  bool loaded;
} level;

/// @brief Struct to define bird
typedef struct bird {
  /// @brief Gravity applied to bird
  float gravity;
  /// @brief Jump
  float jump_speed;
  /// @brief Color of the bird (bits)
  int colorbits;
  /// @brief Actual bird vertical speed (+ equals up, - equals down)
  float act_speed;
  /// @brief Actual bird vertical position relative to map start (not screen)
  float act_position;
  /// @brief Last time processed ms
  long long last_time_ms;
} bird;

/// @brief Struct to define screen
typedef struct screen {
  /// @brief Header height, 0 means auto
  int header_height;
  /// @brief Header width
  int header_width;
  /// @brief Header padding
  int header_padding;
  /// @brief Border color fg and bg (bits)
  int border_color;
  /// @brief Header color fg and bg (bits)
  int header_color;
} screen;

/// @brief Struct to define render settings
typedef struct render_settings {
  /// @brief Screen refresh rate
  int fps;
} render_settings;

int init_screen();
int render_borders();
int render_header_text(int lines, int maxstring,
                       char header_text[lines][maxstring]);
int clear_header(bool full);
int clear_map_area(level *inplvl, bool full);
int print_level_info(level *inplvl, int yoffset);
int render_menu(int option_selected, char nickname[]);
int load_settings();
fbpipe get_pipe(int x, level *inplvl, bool enable, int prevupheight);
int render_pipe(fbpipe *inputp, level *inplvl);
int render_bird(bird *inpb, int xpos, bool show_collision);
bird get_bird(level *inplvl);
int print_header_options(int option_items_n, int maxstrlen,
                         char option_items[option_items_n][maxstrlen],
                         int option_selected, int ypos);
int print_level_options(int option_selected);
int move_pipes(const level *inplvl);
int process_pipes(level *inplvl);
int render_pipes();
int increase_speed(level *inplvl);
bool bird_collision(bird *inpb, int xpos);
int move_bird(bird *bird);
int run_level(level *inplvl, int *status);
int print_game_details(int actlives, int score, level *inplvl, bird *inpb);
level load_level_file(int levelnum);
int render_header_string(char *header_text, int yoff, bool setcolor,
                         bool cl_hdr);
void turn_on_header_color(bool switchbgfg);
void turn_off_header_color(bool switchbgfg);
int render_about_page(int yoffset);
bool render_hof(config_option_t hoff, int yoff, bool dofree,
                char actnickname[64]);
int render_bird_floating(level *inplvl, int degrees);