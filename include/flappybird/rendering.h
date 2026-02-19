// Copyright 2022 <Maros Varchola - mvarchdev>

#ifndef FLAPPYBIRD_RENDERING_H
#define FLAPPYBIRD_RENDERING_H

#include <stdbool.h>
#include <stdint.h>

#include "flappybird/confparser.h"

/// @brief Physics-to-render conversion coefficient (meters to characters).
#define METERTOCHARS 0.5

// Map dimensions in character units.
#define MAPSIZEX 130
#define MAPSIZEY 28
#define MAPMARGIN 1
#define OUTERMARGIN 5
#define BORDERWIDTH 1
#define PIPEWIDTH 3

/// @brief Path to assets folder.
#define ASSETS_FOLDER "./assets"
/// @brief Path to settings folder.
#define SETTINGS_FOLDER ASSETS_FOLDER "/settings"
/// @brief Path to settings file.
#define SETTINGS_FILE SETTINGS_FOLDER "/settings.conf"

/// @brief Menu item count.
#define MENU_ITEM_COUNT 6

// Backward compatibility with existing identifier used across the codebase.
#define menu_inem_n MENU_ITEM_COUNT

/// @brief Struct to define one pipe.
typedef struct fbpipe {
  int pipewidth;
  int position;
  int upheight;
  int downheight;
  long long last_time_moved;
  bool enabled;
} fbpipe;

/// @brief Struct to define one level.
typedef struct level {
  int levelnumber;
  char levelname[50];
  int bgcolor;
  int pipe_color_brd;
  int pipe_color_body;
  float start_speed;
  float speed_increase;
  int minimum_space;
  int maximum_space;
  int minimum_width;
  int maximum_width;
  int minimum_distance;
  int maximum_distance;
  int minimum_distance_space;
  int maximum_distance_space;
  float gravity_multiply;
  float jump_speed;
  int max_lives;
  bool loaded;
} level;

/// @brief Struct to define bird.
typedef struct bird {
  float gravity;
  float jump_speed;
  int colorbits;
  float act_speed;
  float act_position;
  long long last_time_ms;
} bird;

/// @brief Struct to define screen.
typedef struct screen {
  int header_height;
  int header_width;
  int header_padding;
  int border_color;
  int header_color;
} screen;

/// @brief Struct to define render settings.
typedef struct render_settings {
  int fps;
} render_settings;

int init_screen(void);
int render_borders(void);
int render_header_text(int lines, int maxstring, char header_text[lines][maxstring]);
int clear_header(bool full);
int clear_map_area(level *inplvl, bool full);
int print_level_info(level *inplvl, int yoffset);
int render_menu(int option_selected, const char nickname[]);
int load_settings(void);
fbpipe get_pipe(int x, level *inplvl, bool enable, int prevupheight);
int render_pipe(fbpipe *inputp, level *inplvl);
int render_bird(bird *inpb, int xpos, bool show_collision);
bird get_bird(level *inplvl);
int print_header_options(int option_items_n, int maxstrlen,
                         char option_items[option_items_n][maxstrlen], int option_selected,
                         int ypos);
int print_level_options(int option_selected);
int move_pipes(const level *inplvl);
int process_pipes(level *inplvl);
int render_pipes(level *inplvl);
int increase_speed(level *inplvl);
bool bird_collision(bird *inpb, int xpos);
int move_bird(bird *bird);
int run_level(level *inplvl, int *status);
int print_game_details(int actlives, int score, level *inplvl, bird *inpb);
level load_level_file(int levelnum);
int render_header_string(const char *header_text, int yoff, bool setcolor, bool cl_hdr);
void turn_on_header_color(bool switchbgfg);
void turn_off_header_color(bool switchbgfg);
int render_about_page(int yoffset);
bool render_hof(config_option_t hoff, int yoff, bool dofree, const char actnickname[64]);
int render_bird_floating(level *inplvl, int degrees);

#endif  // FLAPPYBIRD_RENDERING_H
