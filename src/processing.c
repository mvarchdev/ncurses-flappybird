// Copyright 2022 <Maros Varchola - mvarchdev>

#include "flappybird/processing.h"

#include <ctype.h>
#include <limits.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "flappybird/common_tools.h"
#include "flappybird/rendering.h"

#define SAVES_FILE "./assets/saves.conf"
#define HALLOFFAME_FILE "./assets/hall_of_fame.conf"

static char active_nickname[64] = {0};

static int safe_tolower(int ch) {
  if (ch < 0 || ch > UCHAR_MAX) {
    return ch;
  }
  return tolower((unsigned char)ch);
}

static int get_last_level(const char *nickname) { return get_int_key_value(SAVES_FILE, nickname); }

static int set_last_level(const char *nickname, int level) {
  return set_int_key_value("./lvltmpfile", SAVES_FILE, nickname, level);
}

static int get_hall_of_fame(const char *nickname, int level) {
  char key[120] = {0};
  snprintf(key, sizeof(key), "%s#lvl_%d#", nickname, level);
  return get_int_key_value(HALLOFFAME_FILE, key);
}

static int set_hall_of_fame(const char *nickname, int score, int level) {
  char key[120] = {0};
  snprintf(key, sizeof(key), "%s#lvl_%d#", nickname, level);
  return set_int_key_value("./hoftmpfile", HALLOFFAME_FILE, key, score);
}

static int process_run_level(level *input_level) {
  if (input_level == NULL) {
    return -1;
  }

  if (!input_level->loaded) {
    char output[60] = {0};
    snprintf(output, sizeof(output), "The level %d does not exist! Exiting...",
             input_level->levelnumber);
    render_header_string(output, 0, true, true);
    refresh();
    msleep(1500);
    flushinp();
    return -1;
  }

  int status = 0;
  set_last_level(active_nickname, input_level->levelnumber);
  int score = run_level(input_level, &status);

  int high_score = get_hall_of_fame(active_nickname, input_level->levelnumber);
  char message[255] = {0};
  if (score > high_score && score > 0) {
    snprintf(message, sizeof(message),
             "GAME OVER! NEW HIGH SCORE FOR THIS LEVEL! %s - YOUR SCORE: %d "
             "[SAVING IT TO HALL OF FAME]",
             active_nickname, score);
    set_hall_of_fame(active_nickname, score, input_level->levelnumber);
  } else {
    snprintf(message, sizeof(message), "GAME OVER! %s - YOUR SCORE: %d", active_nickname, score);
  }

  render_header_string(message, -1, true, true);
  refresh();
  msleep(1500);
  flushinp();

  return score;
}

static int select_level_dialog(void) {
  int selected_option = 0;
  int selected_level = 1;
  int scroll = 0;
  int degrees = 0;

  keypad(stdscr, true);
  while (true) {
    bool redraw = false;
    level level_preview = load_level_file(selected_level);
    int max_scroll = print_level_info(&level_preview, scroll);
    print_level_options(selected_option);

    refresh();
    flushinp();
    timeout(0);

    while (true) {
      degrees = render_bird_floating(&level_preview, degrees);
      int ch = getch();
      switch (ch) {
        case KEY_RIGHT:
          if (selected_option < 5) {
            selected_option++;
          }
          redraw = true;
          break;
        case KEY_LEFT:
          if (selected_option > 0) {
            selected_option--;
          }
          redraw = true;
          break;
        case '\n':
          redraw = true;
          switch (selected_option) {
            case 0:
              return selected_level;
            case 1:
              if (scroll < max_scroll) {
                scroll++;
              }
              break;
            case 2:
              if (scroll > 0) {
                scroll--;
              }
              break;
            case 3:
              if (load_level_file(selected_level + 1).loaded) {
                selected_level++;
              }
              break;
            case 4:
              if (selected_level > 1 && load_level_file(selected_level - 1).loaded) {
                selected_level--;
              }
              break;
            case 5:
              keypad(stdscr, false);
              return 0;
            default:
              break;
          }
          break;
        default:
          break;
      }

      if (redraw) {
        break;
      }
    }
  }
}

static int show_about_page(void) {
  int scroll = 0;
  timeout(-1);
  keypad(stdscr, true);
  render_header_string("This is \"about this game\" page, to exit, press 'b'", 1, true, true);

  while (true) {
    int max_scroll = render_about_page(scroll);
    refresh();
    flushinp();
    int ch = getch();

    if (ch == KEY_UP) {
      if (scroll > 0) {
        scroll--;
      }
      continue;
    }
    if (ch == KEY_DOWN) {
      if (scroll < max_scroll) {
        scroll++;
      }
      continue;
    }
    if (safe_tolower(ch) == 'b') {
      timeout(0);
      keypad(stdscr, false);
      return 0;
    }
  }
}

static int show_hof(void) {
  int scroll = 0;
  timeout(-1);
  keypad(stdscr, true);
  render_header_string("This is \"HALL OF FAME\" page, to exit, press 'b'", 1, true, true);

  config_option_t hof = read_config_file(HALLOFFAME_FILE);
  while (true) {
    bool scroll_allowed = render_hof(hof, scroll, false, active_nickname);
    refresh();
    flushinp();

    int ch = getch();
    if (ch == KEY_UP) {
      if (scroll > 0) {
        scroll--;
      }
      continue;
    }
    if (ch == KEY_DOWN) {
      if (scroll_allowed) {
        scroll++;
      }
      continue;
    }
    if (safe_tolower(ch) == 'b') {
      timeout(0);
      keypad(stdscr, false);
      free_config_options(hof);
      return 0;
    }
  }
}

static void request_nickname(void) {
  while (true) {
    render_header_string("Please write here your nickname (max 63 chars): ", 1, true, true);
    refresh();
    echo();
    curs_set(1);
    flushinp();
    timeout(-1);
    turn_on_header_color(true);
    getnstr(active_nickname, (int)(sizeof(active_nickname) - 1));
    turn_off_header_color(true);
    curs_set(0);
    noecho();
    if (strcmp("", active_nickname) != 0) {
      break;
    }
    render_header_string("Your nick cannot be empty!", 1, true, true);
    refresh();
    msleep(1500);
  }

  char message[120] = {0};
  snprintf(message, sizeof(message), "Your nickname is now: %s", active_nickname);
  render_header_string(message, 1, true, true);
  refresh();
  msleep(1500);
}

static int process_option(int option) {
  switch (option) {
    case 0: {
      int last_level = get_last_level(active_nickname);
      if (last_level > 0) {
        level loaded_level = load_level_file(last_level);
        if (!loaded_level.loaded) {
          render_header_string("The level that is saved, does not exist! Setting to level 1.", 0,
                               true, true);
          refresh();
          msleep(1500);
          flushinp();
          set_last_level(active_nickname, 1);
        } else {
          process_run_level(&loaded_level);
        }
      } else {
        render_header_string("You are playing this game for the first time, running level 1", 0,
                             true, true);
        refresh();
        msleep(1500);
        flushinp();
        level loaded_level = load_level_file(1);
        process_run_level(&loaded_level);
      }
      break;
    }
    case 1: {
      int selected_level = select_level_dialog();
      if (selected_level > 0) {
        level loaded_level = load_level_file(selected_level);
        process_run_level(&loaded_level);
      }
      break;
    }
    case 2:
      request_nickname();
      break;
    case 3:
      show_hof();
      break;
    case 4:
      show_about_page();
      break;
    default:
      break;
  }
  return 0;
}

static int run_menu(void) {
  int selected_option = 0;

  while (true) {
    keypad(stdscr, true);
    render_menu(selected_option, active_nickname);
    refresh();
    flushinp();
    timeout(-1);

    int ch = getch();
    switch (ch) {
      case KEY_RIGHT:
        if (selected_option < menu_inem_n - 1) {
          selected_option++;
        }
        break;
      case KEY_LEFT:
        if (selected_option > 0) {
          selected_option--;
        }
        break;
      case '\n':
        if (selected_option == 5) {
          keypad(stdscr, false);
          return 1;
        }
        process_option(selected_option);
        break;
      default:
        break;
    }
  }
}

int run_game(void) {
  request_nickname();
  run_menu();
  return 0;
}
