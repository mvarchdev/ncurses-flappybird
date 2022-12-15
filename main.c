#include "rendering.h"
#include "processing.h"
#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <stdlib.h>

int main()
{
    srand(time(NULL));
    init_screen();
    run_game();
    endwin();
}