// Copyright 2022 <Maros Varchola - mvarchdev>

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "processing.h"
#include "rendering.h"

int main() {
  srand(time(NULL));
  init_screen();
  run_game();
  endwin();
}