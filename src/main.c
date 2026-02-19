// Copyright 2022 <Maros Varchola - mvarchdev>

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "flappybird/processing.h"
#include "flappybird/rendering.h"

int main(void) {
  srand((unsigned int)time(NULL));
  if (init_screen() != 0) {
    fprintf(stderr, "Failed to initialize screen.\n");
    return EXIT_FAILURE;
  }

  run_game();
  endwin();
  return EXIT_SUCCESS;
}
