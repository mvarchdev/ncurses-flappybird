#include <ncurses.h>
#include <stdlib.h>
#include "rendering.h"
#include <string.h>
#include <ctype.h>
#include "common_tools.h"
#include <time.h>
#include <unistd.h>
#include <math.h>

/// @brief Up left border character
#define UPLEFTBORDER '#'
/// @brief Up right border character
#define UPRIGHTBORDER '#'
/// @brief Down left border character
#define DOWNLEFTBORDER '#'
/// @brief Down right border character
#define DOWNRIGHTBORDER '#'
/// @brief Border vertical character
#define VERTBORDER '-'
/// @brief Border horizontal character
#define HORBORDER '|'
/// @brief Up, down, right join border character
#define UPDOWNRIGHTBORDER '#'
/// @brief Up, down, left join border character
#define UPDOWNLEFTBORDER '#'

/// @brief Pipe border vertical character
#define PIPEVERT '_'
/// @brief Pipe border horizontal character
#define PIPEHORI '|'
/// @brief Pipe body character
#define PIPEBODY '#'
/// @brief End-of-pipe character
#define PIPEEND '*'
/// @brief Pipehole end width
#define PIPEHOLE_END_WIDTH 2
/// @brief Pipehole end height
#define PIPEHOLE_END_HEIGHT 1

/// @brief Default bird x offset
#define BIRDOFFX 30

/// @brief Maximal header string for game details
#define MAXHEADERSTRING 40

/// @brief Maximum of piped that can be rendered at once
#define MAX_PIPES 30

/// @brief Variable to save pipes to render
fbpipe pipe_array[MAX_PIPES] = {0};

/// @brief Loaded settings for screen
screen act_screen = {0};
/// @brief Loaded render settings
render_settings act_rndsett = {0};

/// @brief Speed in chars/s
float act_speed_chars = 0;

/// @brief Variable for computing speed increase
long long last_time = 0;

/// @brief Gravity constant
float gravity_constant = 9.8;
/// @brief If no gravity multiply is set, then default will be used
float def_grav_multiply = 1;
/// @brief Default speed increase
float def_speed_incr = 0.125;
/// @brief Total horizontal size of full render area
int xsize = 0;
/// @brief Total vertical size of full render area
int ysize = 0;
/// @brief Total horizontal offset of map render area
int mapoffsx = 0;
/// @brief Total vertical offset of map render area
int mapoffsy = 0;
/// @brief Total horizontal offset of header render area
int headeroffsx = 0;
/// @brief Total vertical offset of header render area
int headeroffsy = 0;
/// @brief Total horizontal size/width of header
int headersizex = 0;

/// @brief Maximum string length of menu item
#define menu_item_maxstrlen 30

/// @brief Total vertical offset of map render area
char menu_items[menu_inem_n + 1][menu_item_maxstrlen] = {
    "Start (where you ended)",
    "Select level",
    "Change nickname",
    "Show hall of fame",
    "About",
    "Exit",
};

/// @brief Function checks if intensity bit is set
/// @param fg Colorbits input
/// @return True, if intensity bit is set, else false
int is_bold_bits(int fg)
{
    int i;

    i = 1 << 3;
    return (i & fg);
}

/// @brief Get uniqe color pairnum
/// @param fg Foreground color bits
/// @param bg Background color bits
/// @return Uniqe pairnum
int get_col_pairnum(int fg, int bg)
{
    int B = 0, bbb = 0, ffff = 0;

    B = 1 << 7;
    bbb = (7 & bg) << 4;
    ffff = 7 & fg;

    return (B | bbb | ffff);
}

/// @brief Sets the actual terminal color based on uniqe colorpair
/// @param fg Foreground color bits
/// @param bg Background color bits
void setcolor_bits(int fg, int bg)
{
    attron(COLOR_PAIR(get_col_pairnum(fg, bg)));
    if (is_bold_bits(fg))
    {
        attron(A_BOLD);
    }
}

/// @brief Sets the actual terminal color based on uniqe colorpair
/// @param fg Foreground color bits
/// @param bg Background color bits
void unsetcolor_bits(int fg, int bg)
{
    attroff(COLOR_PAIR(get_col_pairnum(fg, bg)));
    if (is_bold_bits(fg))
    {
        attroff(A_BOLD);
    }
}

/// @brief Returns if input colorname/string indicates intensity bit set
/// @param colname String with colorname
/// @return True, if it is with intensity set
bool is_bold(char colname[])
{
    if (!colname)
        return false;

    for (int i = 0; i < strlen(colname); i++)
        colname[i] = toupper(colname[i]);

    if (strstr(colname, "B_"))
        return true;

    return false;
}

/// @brief Function will match colorname to native color number
/// @param colname String with colorname
/// @return Native color number, -1 if not existing
short string_to_color(char colname[])
{
    if (!colname)
        return false;

    for (int i = 0; i < strlen(colname); i++)
        colname[i] = (colname[i]);

    if (strstr(colname, "BLACK"))
        return COLOR_BLACK;
    else if (strstr(colname, "RED"))
        return COLOR_RED;
    else if (strstr(colname, "GREEN"))
        return COLOR_GREEN;
    else if (strstr(colname, "YELLOW"))
        return COLOR_YELLOW;
    else if (strstr(colname, "BLUE"))
        return COLOR_BLUE;
    else if (strstr(colname, "MAGENTA"))
        return COLOR_MAGENTA;
    else if (strstr(colname, "CYAN"))
        return COLOR_CYAN;
    else if (strstr(colname, "WHITE"))
        return COLOR_WHITE;

    return -1;
}

/// @brief Function will return oposite of native color
/// @param col Native color number
/// @return Native number of oposite color
short opposit_col(short col)
{
    switch (col)
    {
    case COLOR_BLACK:
        return (COLOR_WHITE);
    case COLOR_RED:
        return (COLOR_GREEN);
    case COLOR_GREEN:
        return (COLOR_RED);
    case COLOR_YELLOW:
        return (COLOR_CYAN);
    case COLOR_BLUE:
        return (COLOR_RED);
    case COLOR_MAGENTA:
        return (COLOR_BLACK);
    case COLOR_CYAN:
        return (COLOR_YELLOW);
    case COLOR_WHITE:
        return (COLOR_BLACK);
    }

    return -1;
}

/// @brief Will convert native color to foreground bitscolor
/// @param color Native color number
/// @param bold If color intensity bit should be set
/// @return Foreground bitscolor based on input
int native_to_bitscolor(short color, bool bold)
{
    int bitscolor = bold ? (1 << 3) : 0;
    switch (color)
    { /* RGB */
    case COLOR_BLACK:
        return bitscolor | 0; /* 000 */
    case COLOR_BLUE:
        return bitscolor | 1; /* 001 */
    case COLOR_GREEN:
        return bitscolor | 2; /* 010 */
    case COLOR_CYAN:
        return bitscolor | 3; /* 011 */
    case COLOR_RED:
        return bitscolor | 4; /* 100 */
    case COLOR_MAGENTA:
        return bitscolor | 5; /* 101 */
    case COLOR_YELLOW:
        return bitscolor | 6; /* 110 */
    case COLOR_WHITE:
        return bitscolor | 7; /* 111 */
    }

    return -1;
}

/// @brief Will convert native color to background bitscolor
/// @param color Native color number
/// @return Background bitscolor based on input
int native_to_bitscolor_bg(short color)
{
    int bcolor = native_to_bitscolor(color, false);
    if (bcolor > -1)
        return (7 & bcolor) << 4;
    return bcolor;
}

/// @brief Will shift background color to foreground from input
/// @param bg Bitscolor with background color set
/// @return Bitscolor shifted from bg to fg
int bitscolor_bg_to_fg(int bg)
{
    return (bg >> 4) & 7;
}

/// @brief Will convert bitscolor to native color
/// @param color Bitscolor
/// @return Native number of color
short bits_to_native_color(int color)
{
    switch (7 & color)
    {       /* RGB */
    case 0: /* 000 */
        return (COLOR_BLACK);
    case 1: /* 001 */
        return (COLOR_BLUE);
    case 2: /* 010 */
        return (COLOR_GREEN);
    case 3: /* 011 */
        return (COLOR_CYAN);
    case 4: /* 100 */
        return (COLOR_RED);
    case 5: /* 101 */
        return (COLOR_MAGENTA);
    case 6: /* 110 */
        return (COLOR_YELLOW);
    case 7: /* 111 */
        return (COLOR_WHITE);
    }

    return -1;
}

/// @brief Will init all uniqe colorpairs
void init_colorpairs()
{
    int fg, bg;
    int colorpair;

    for (bg = 0; bg <= 7; bg++)
        for (fg = 0; fg <= 7; fg++)
        {
            colorpair = get_col_pairnum(fg, bg);
            init_pair(colorpair, bits_to_native_color(fg), bits_to_native_color(bg));
        }
}

/// @brief Will initialize screen and compute base offsets and sizes
/// @return Error code
int init_screen()
{
    load_settings();
    initscr();
    curs_set(0);
    start_color();
    init_colorpairs();
    noecho();

    xsize = (2 * OUTERMARGIN) + (2 * BORDERWIDTH) + (2 * MAPMARGIN) + MAPSIZEX;
    ysize = (2 * OUTERMARGIN) + (3 * BORDERWIDTH) + (2 * MAPMARGIN) + MAPSIZEY + act_screen.header_height + (act_screen.header_padding * 2);
    mapoffsx = OUTERMARGIN + BORDERWIDTH + MAPMARGIN;
    mapoffsy = OUTERMARGIN + BORDERWIDTH * 2 + act_screen.header_padding * 2 + act_screen.header_height + MAPMARGIN;
    headeroffsx = OUTERMARGIN + BORDERWIDTH + act_screen.header_padding;
    headeroffsy = OUTERMARGIN + BORDERWIDTH + act_screen.header_padding;

    headersizex = MAPSIZEX - 2 * act_screen.header_padding;

    act_screen.header_width = xsize - (OUTERMARGIN * 2) - (BORDERWIDTH * 2) - (act_screen.header_padding * 2);

    resizeterm(ysize, xsize);
    render_borders();

    refresh();
    return 0;
}

/// @brief Change speed of bird based jump speed
/// @param inpb Bird struct pointer to use
void jump_bird(bird *inpb)
{
    if (!inpb)
        return;

    if (inpb->act_speed < 0)
    {
        if ((inpb->act_speed - inpb->jump_speed) < -1.5 * inpb->jump_speed)
            inpb->act_speed = -1.5 * inpb->jump_speed;
        else
            inpb->act_speed -= inpb->jump_speed;
    }
    else
        inpb->act_speed = -inpb->jump_speed;
}

/// @brief Will print actual running game details to the header area
/// @param actlives Actual lives
/// @param score Actual score
/// @param inplvl Level struct pointer
/// @param inpb Bird struct pointer
/// @return Error code
int print_game_details(int actlives, int score, level *inplvl, bird *inpb)
{
    if (!inplvl || !inpb)
        return -1;

    int i = 0;
    char headerinp[7][MAXHEADERSTRING] = {0};
    sprintf(headerinp[i++], "Level name: %s", inplvl->levelname);
    sprintf(headerinp[i++], "Score: %d", score);
    sprintf(headerinp[i++], "Lives [Actual / Max]: %d / %d", actlives, inplvl->max_lives);
    sprintf(headerinp[i++], "Speed: %.3f [char/s]", act_speed_chars);
    sprintf(headerinp[i++], "Bird speed: %.3f [char/s]", inpb->act_speed);
    i++;
    sprintf(headerinp[i++], "Pause: p | End game: e");
    return render_header_text(7, MAXHEADERSTRING, headerinp);
}

/// @brief Game paused dialog
/// @return Error code
int game_paused_dialog()
{
    char headerinp[1][60] = {0};
    sprintf(headerinp[0], "GAME PAUSED - PRESS 'p' TO CONTINUE OR 'e' TO END GAME");
    render_header_text(1, 60, headerinp);
    timeout(-1);
    while (true)
    {
        int ch = getch();
        if (ch == EOF)
            NULL;
        else if (tolower(ch) == 'p')
            return 0;
        else if (tolower(ch) == 'e')
            return 1;
    }
}

/// @brief Force set of lastms rendered variable for enabled pipes
/// @param timems Time to set in ms
void update_last_ms_pipes(long long timems)
{
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (!pipe_array[i].enabled)
            continue;
        pipe_array[i].last_time_moved = timems;
    }
}

/// @brief Collision dialog
/// @param actlives Actual lives
/// @param score Actual score
/// @return
int colision_dialog(int actlives, int score)
{
    char headerinp[4][90] = {0};
    sprintf(headerinp[0], "BANG! You crashed into the pipe or you fell down, you have %d more lives left.", actlives);
    sprintf(headerinp[1], "YOUR ACTUAL SCORE IS: %d", score);

    sprintf(headerinp[3], "Do you want to try again (press 't') or end the game (press 'e') ?");
    render_header_text(4, 90, headerinp);
    timeout(-1);
    while (true)
    {
        int ch = getch();
        if (ch == EOF)
            NULL;
        else if (tolower(ch) == 't')
            return 0;
        else if (tolower(ch) == 'e')
            return 1;
    }
}

/// @brief Will disable all enabled pipes
void clear_all_pipes()
{
    for (int i = 0; i < MAX_PIPES; i++)
        if (pipe_array[i].enabled)
            pipe_array[i].enabled = false;
}

/// @brief Function to run level
/// @param inplvl Pointer to level to use
/// @param status Pointer to status output
/// @return Score
int run_level(level *inplvl, int *status)
{
    if (!inplvl)
        return -1;

    int actlives = inplvl->max_lives, score = 0;
    act_speed_chars = inplvl->start_speed * METERTOCHARS;
    int statustmp = 0;
    bird usebird = {0};
    if (!status)
        status = &statustmp;

    while (actlives != 0)
    {
        clear_all_pipes();
        usebird = get_bird(inplvl);
        last_time = 0;

        timeout(0);
        while (true)
        {

            int ch = getch();
            if (ch == EOF)
                NULL;
            else
            {
                if (ch == ' ')
                    jump_bird(&usebird);
                else if (tolower(ch) == 'e')
                {
                    *status = 1;
                    break;
                }
                else if (tolower(ch) == 'p')
                {
                    if (game_paused_dialog() == 1)
                    {
                        *status = 1;
                        break;
                    }
                    timeout(0);
                    usebird.last_time_ms = timeInMilliseconds();
                    update_last_ms_pipes(usebird.last_time_ms);
                    last_time = 0;
                }
                flushinp();
            }
            print_game_details(actlives, score, inplvl, &usebird);
            move_bird(&usebird);
            clear_map_area(inplvl, true);
            render_pipes(inplvl);
            if (bird_collision(&usebird, BIRDOFFX) || usebird.act_position >= MAPSIZEY - 1)
                break;

            render_bird(&usebird, BIRDOFFX, false);
            score += move_pipes(inplvl);
            process_pipes(inplvl);
            // last_time = timeInMilliseconds();
            increase_speed(inplvl);
            refresh();

            msleep(1000 / act_rndsett.fps);

            // usleep(1000);
        }
        if (*status == 1)
            break;
        actlives--;
        render_bird(&usebird, BIRDOFFX, true);
        if (actlives > 0)
            if (colision_dialog(actlives, score) == 1)
            {
                *status = 1;
                break;
            }
    }

    flushinp();
    timeout(-1);
    return score;
}

/// @brief Will render all borders
/// @return Error code
int render_borders()
{
    setcolor_bits(act_screen.border_color, bitscolor_bg_to_fg(act_screen.border_color));
    for (int i = 0; i < BORDERWIDTH; i++)
    {
        mvaddch(OUTERMARGIN + i, OUTERMARGIN + i, UPLEFTBORDER);
        mvaddch(OUTERMARGIN + i, xsize - OUTERMARGIN - 1 - i, UPRIGHTBORDER);
        mvaddch(ysize - OUTERMARGIN - 1 - i, OUTERMARGIN + i, DOWNLEFTBORDER);
        mvaddch(ysize - OUTERMARGIN - 1 - i, xsize - OUTERMARGIN - 1 - i, DOWNRIGHTBORDER);

        for (int z = OUTERMARGIN + i + 1; z < xsize - OUTERMARGIN - i - 1; z++)
        {
            mvaddch(OUTERMARGIN + i, z, VERTBORDER);
            mvaddch(ysize - OUTERMARGIN - i - 1, z, VERTBORDER);
        }

        for (int z = OUTERMARGIN + i + 1; z < ysize - OUTERMARGIN - i - 1; z++)
        {
            if (i + 1 == BORDERWIDTH && (((1 * OUTERMARGIN) + (2 * BORDERWIDTH) + act_screen.header_height + (act_screen.header_padding * 2) - 1) == z))
            {
                mvaddch(z, OUTERMARGIN + i, UPDOWNRIGHTBORDER);
                for (int g = OUTERMARGIN + i + 1; g < xsize - OUTERMARGIN - 1; g++)
                    mvaddch(z, g, VERTBORDER);
                mvaddch(z, xsize - OUTERMARGIN - 1 - i, UPDOWNLEFTBORDER);
            }
            else
            {
                mvaddch(z, OUTERMARGIN + i, HORBORDER);
                mvaddch(z, xsize - OUTERMARGIN - 1 - i, HORBORDER);
            }
        }
    }
    unsetcolor_bits(act_screen.border_color, bitscolor_bg_to_fg(act_screen.border_color));
    return 0;
}

/// @brief Common function to render header text
/// @param lines How much lines is in header_text
/// @param maxstring String length of one line in header_text
/// @param header_text 2D Array of chars (More string lines)
/// @return Error code
int render_header_text(int lines, int maxstring, char header_text[lines][maxstring])
{
    setcolor_bits(act_screen.header_color, bitscolor_bg_to_fg(act_screen.header_color));
    clear_header(true);

    int yoff = 0;
    for (int i = 0; i < lines && i < act_screen.header_height; i++)
        yoff += render_header_string(header_text[i], yoff, false, false);
    unsetcolor_bits(act_screen.header_color, bitscolor_bg_to_fg(act_screen.header_color));
    return 0;
}

/// @brief Will set header color on
/// @param switchbgfg Switch fg to bg and viceversa
void turn_on_header_color(bool switchbgfg)
{
    if (switchbgfg)
        setcolor_bits(bitscolor_bg_to_fg(act_screen.header_color), act_screen.header_color);
    else
        setcolor_bits(act_screen.header_color, bitscolor_bg_to_fg(act_screen.header_color));
}

/// @brief Will set header color off
/// @param switchbgfg Switch fg to bg and viceversa
void turn_off_header_color(bool switchbgfg)
{
    if (switchbgfg)
        unsetcolor_bits(bitscolor_bg_to_fg(act_screen.header_color), act_screen.header_color);
    else
        unsetcolor_bits(act_screen.header_color, bitscolor_bg_to_fg(act_screen.header_color));
}

/// @brief Prints string and computes lines based on header width
/// @param header_text Input text that needs to be print out
/// @param yoff y offset
/// @param setcolor If true, it will set color based on the settings
/// @param cl_hdr If true, it will clear header before print out
/// @return How many lines this function have printed out
int render_header_string(char header_text[], int yoff, bool setcolor, bool cl_hdr)
{
    if (yoff == -1)
        yoff = act_screen.header_height / 2;

    if (setcolor)
        setcolor_bits(act_screen.header_color, bitscolor_bg_to_fg(act_screen.header_color));

    if (cl_hdr)
        clear_header(true);

    int parts = 1;
    if (strlen(header_text) > headersizex)
        parts += strlen(header_text) / headersizex;

    int i = 0;
    for (; i < parts; i++)
        mvprintw(headeroffsy + yoff + i, headeroffsx, "%.*s", headersizex, (headersizex * i) + header_text);
    if (setcolor)
        unsetcolor_bits(act_screen.header_color, bitscolor_bg_to_fg(act_screen.header_color));
    return i;
}

/// @brief Clear map area
/// @param inplvl Input level, if null, colors will not be used
/// @param full If true, will clear also margins
/// @return Error code
int clear_map_area(level *inplvl, bool full)
{
    int bgoppcolor = 0;
    if (inplvl)
    {
        bgoppcolor = native_to_bitscolor(opposit_col(bits_to_native_color(bitscolor_bg_to_fg(inplvl->bgcolor))), true);
        setcolor_bits(bgoppcolor, bitscolor_bg_to_fg(inplvl->bgcolor));
    }

    for (int y = 0; y < MAPSIZEY + (full ? MAPMARGIN * 2 : 0); y++)
        for (int x = 0; x < MAPSIZEX + (full ? MAPMARGIN * 2 : 0); x++)
            mvaddch(y + mapoffsy - (full ? MAPMARGIN : 0), x + mapoffsx - (full ? MAPMARGIN : 0), ' ');

    if (inplvl)
        unsetcolor_bits(bgoppcolor, bitscolor_bg_to_fg(inplvl->bgcolor));

    return 0;
}

/// @brief Clear header area
/// @param full If true, will clear also padding area
/// @return Error code
int clear_header(bool full)
{
    for (int y = 0; y < act_screen.header_height + (full ? act_screen.header_padding * 2 : 0); y++)
        for (int x = 0; x < act_screen.header_width + (full ? act_screen.header_padding * 2 : 0); x++)
            mvaddch(y + headeroffsy - (full ? act_screen.header_padding : 0), x + headeroffsx - (full ? act_screen.header_padding : 0), ' ');
    return 0;
}

/// @brief Render menu
/// @param option_selected Which option shoud be highlighted
/// @param nickname Nickname to be shown
/// @return Error code
int render_menu(int option_selected, char nickname[])
{
    int err = print_header_options(menu_inem_n, 30, menu_items, option_selected, (act_screen.header_height - 1) / 2);
    if (err != 0)
        return err;
    char namestr[120] = {0};
    sprintf(namestr, "Welcome, %s!", nickname);
    return render_header_string(namestr, 0, true, false);
}

/// @brief Will render 'About this game' page
/// @param yoffset Scroll offset
/// @return Maximal allowed scroll
int render_about_page(int yoffset)
{
    char aboutinfo[10][100] = {0};

    int lineidx = 0;
    sprintf(aboutinfo[lineidx++], "Welcome to this game named Flappy Bird - created in ncurses framework!");
    sprintf(aboutinfo[lineidx++], "This game was created by Maros Varchola as school project, but now, as you can see.");
    sprintf(aboutinfo[lineidx++], "It is used as part of my portfolio.");

    sprintf(aboutinfo[lineidx++], "If you have any ideas on how to improve this game, do not be shy and try a PR.");
    sprintf(aboutinfo[lineidx++], "I believe it will be very interesting change/feature!");

    sprintf(aboutinfo[lineidx++], "In this game, the real physics calculations are used, so even gravity constant is set here!");

    sprintf(aboutinfo[lineidx++], "If you have any questions, write me on marosvarchola@sunray.sk !");
    lineidx++;
    sprintf(aboutinfo[lineidx++], "ENJOY THE GAME!");

    int bgoppcolor = native_to_bitscolor(opposit_col(bits_to_native_color(bitscolor_bg_to_fg(act_screen.header_color))), true);
    setcolor_bits(bgoppcolor, bitscolor_bg_to_fg(act_screen.header_color));
    clear_map_area(NULL, true);

    int lineidxcounter = yoffset, i = 0;
    for (; lineidxcounter < lineidx && i < MAPSIZEY - 1; i++)
    {
        mvprintw(mapoffsy + i, mapoffsx, aboutinfo[lineidxcounter]);
        lineidxcounter++;
    }
    if (i >= MAPSIZEY - 1 && lineidxcounter < lineidx)
        mvprintw(mapoffsy + i - 1, mapoffsx, "There is more! Scroll down! (arrows up/down)");

    unsetcolor_bits(bgoppcolor, bitscolor_bg_to_fg(act_screen.header_color));

    if (i >= MAPSIZEY - 1 && lineidxcounter < lineidx)
        return i - MAPSIZEY;

    return 0;
}

/// @brief Get random pipe based on level
/// @param x x offset
/// @param inplvl Level struct pointer to use
/// @param enable If enable pipe
/// @param prevupheight Height of previous pipe
/// @return Generated pipe struct
fbpipe get_pipe(int x, level *inplvl, bool enable, int prevupheight)
{
    fbpipe newpipe = {0};

    if (!inplvl)
        return newpipe;

    newpipe.pipewidth = rand_gen(inplvl->minimum_width, inplvl->maximum_width);

    int holeheight = rand_gen(inplvl->minimum_space, inplvl->maximum_space);
    int full_allowed_height = MAPSIZEY - ((PIPEHOLE_END_HEIGHT + 1) * 2) - holeheight;

    int minheight = 1, maxheight = full_allowed_height - 1;

    if (prevupheight > 0)
    {
        int useinterval = rand_gen(0, 1);
        if (useinterval == 0)
        {
            if (prevupheight - inplvl->maximum_distance_space <= 0)
                useinterval = 1;
        }
        else if (prevupheight + inplvl->minimum_distance_space >= full_allowed_height - 1)
            useinterval = 0;

        if (useinterval == 0)
        {
            minheight = prevupheight - inplvl->maximum_distance_space;
            maxheight = prevupheight - inplvl->minimum_distance_space;
        }
        else
        {
            minheight = prevupheight + inplvl->minimum_distance_space;
            maxheight = prevupheight + inplvl->maximum_distance_space;
        }

        if (minheight <= 0)
            minheight = 1;
        else if (minheight >= full_allowed_height - 1)
            minheight = full_allowed_height - 1;

        if (maxheight >= full_allowed_height - 1)
            maxheight = full_allowed_height - 1;
        else if (maxheight <= 0)
            maxheight = minheight;
    }

    if (minheight == maxheight)
        newpipe.upheight = prevupheight;
    else
        newpipe.upheight = rand_gen(minheight, maxheight);

    newpipe.downheight = full_allowed_height - newpipe.upheight;

    while (newpipe.downheight < 1)
        newpipe.downheight++;

    newpipe.position = x;
    newpipe.enabled = enable;
    newpipe.last_time_moved = timeInMilliseconds();

    return newpipe;
}

/// @brief Checks if coordinates are in map area
/// @param y y coordinate
/// @param x x coordinate
/// @return True if cordinates are in map area, else false
bool check_in_map_ok(int y, int x)
{
    if (y < 0 || y >= MAPSIZEY || x < 0 || x >= MAPSIZEX)
        return false;
    return true;
}

/// @brief Add character to map area
/// @param y y coordinate
/// @param x x coordinate
/// @param inp Character to add
/// @return Error code
int addch_maparea(int y, int x, const chtype inp)
{
    if (check_in_map_ok(y, x))
        return mvaddch(mapoffsy + y, mapoffsx + x, inp);

    return 0;
}

/// @brief Add character to pipe border
/// @param y y coordinate
/// @param x x coordinate
/// @param inp Character to use
/// @param inplvl level struct pointer to use
/// @return Error code
int addch_pipe_border(int y, int x, const chtype inp, level *inplvl)
{
    if (!inplvl)
        return -1;

    unsetcolor_bits(inplvl->pipe_color_body, bitscolor_bg_to_fg(inplvl->pipe_color_body));
    setcolor_bits(inplvl->pipe_color_brd, bitscolor_bg_to_fg(inplvl->pipe_color_brd));
    return addch_maparea(y, x, inp);
}

/// @brief Add character to pipe body
/// @param y y coordinate
/// @param x x coordinate
/// @param inp Character to use
/// @param inplvl level struct pointer to use
/// @return Error code
int addch_pipe_body(int y, int x, const chtype inp, level *inplvl)
{
    if (!inplvl)
        return -1;

    unsetcolor_bits(inplvl->pipe_color_brd, bitscolor_bg_to_fg(inplvl->pipe_color_brd));
    setcolor_bits(inplvl->pipe_color_body, bitscolor_bg_to_fg(inplvl->pipe_color_body));
    return addch_maparea(y, x, inp);
}

/// @brief Will render bird normally, except there is collision, then it will switch fg and bg color
/// @param y y coordinate
/// @param x x coordinate
/// @param inpch Character to add
/// @param inpb Bird struct to use
/// @param showcol If true, then collision will be checked and printed
void render_bird_collision(int y, int x, chtype inpch, bird *inpb, bool showcol)
{
    if (!showcol)
    {
        mvaddch(y, x, inpch);
        return;
    }

    if ((mvinch(y, x) & 255) != ' ')
        setcolor_bits(bitscolor_bg_to_fg(inpb->colorbits), inpb->colorbits);
    mvaddch(y, x, inpch);
    if ((mvinch(y, x) & 255) != ' ')
    {
        unsetcolor_bits(bitscolor_bg_to_fg(inpb->colorbits), inpb->colorbits);
        setcolor_bits(inpb->colorbits, bitscolor_bg_to_fg(inpb->colorbits));
    }
}

/// @brief Will render bird in map area
/// @param inpb Bird structure pointer to use
/// @param xpos x position where bird should be rendered
/// @param show_collision If true, will render also collision
/// @return Error code
int render_bird(bird *inpb, int xpos, bool show_collision)
{
    int xcenter = mapoffsx + xpos;
    int ycenter = mapoffsy + inpb->act_position;

    setcolor_bits(inpb->colorbits, bitscolor_bg_to_fg(inpb->colorbits));
    if (ycenter - 1 >= mapoffsy && ycenter - 1 < (mapoffsy + MAPSIZEY))
        render_bird_collision(ycenter - 1, xcenter, '\\', inpb, show_collision);

    if (ycenter >= mapoffsy && ycenter < (mapoffsy + MAPSIZEY))
    {
        render_bird_collision(ycenter, xcenter - 2, '|', inpb, show_collision);
        render_bird_collision(ycenter, xcenter + 3, '|', inpb, show_collision);
        render_bird_collision(ycenter, xcenter + 2, '*', inpb, show_collision);
        for (size_t i = 0; i < 3; i++)
            render_bird_collision(ycenter, xcenter - 1 + i, '#', inpb, show_collision);
    }
    if (ycenter + 1 >= mapoffsy && ycenter + 1 < (mapoffsy + MAPSIZEY))
        render_bird_collision(ycenter + 1, xcenter, '/', inpb, show_collision);

    unsetcolor_bits(inpb->colorbits, bitscolor_bg_to_fg(inpb->colorbits));
    return 0;
}

/// @brief Process/Move bird
/// @param bird Bird structure pointer to use
/// @return Error code
int move_bird(bird *bird)
{
    if (!bird)
        return -1;

    if (bird->last_time_ms == 0)
    {
        bird->last_time_ms = timeInMilliseconds();
        return 0;
    }

    long long act_time = timeInMilliseconds();
    long long diff_time = act_time - bird->last_time_ms;
    float seconds = ((float)diff_time / 1000);
    float next_pos = bird->act_position + (bird->act_speed * METERTOCHARS) * seconds;
    if (next_pos >= MAPSIZEY)
    {
        bird->last_time_ms = act_time;
        bird->act_speed = 0;
        bird->act_position = MAPSIZEY - 1;
        return 0;
    }
    else if (next_pos <= 0)
    {
        bird->last_time_ms = act_time;
        bird->act_speed = bird->gravity * seconds;
        bird->act_position = 0;
        return 0;
    }

    bird->act_position = next_pos;
    bird->act_speed += bird->gravity * seconds;
    bird->last_time_ms = act_time;

    return 0;
}

/// @brief Will render single pipe
/// @param inputp Pipe struct pointer
/// @param inplvl Level struct pointer
/// @return Error code
int render_pipe(fbpipe *inputp, level *inplvl)
{
    if (!inputp || !inplvl)
        return -1;

    if (inputp->position - 2 >= MAPSIZEX)
        return -1;

    // RENDER UPPER PIPE
    for (int y = 0; y < inputp->upheight && inputp->position < MAPSIZEX; y++)
    {
        addch_pipe_border(y, inputp->position, PIPEHORI, inplvl);
        for (int x = 0; x < inputp->pipewidth && inputp->position + x + 1 < MAPSIZEX; x++)
            addch_pipe_body(y, inputp->position + x + 1, PIPEBODY, inplvl);
        if ((inputp->position + inputp->pipewidth + 1) < MAPSIZEX)
            addch_pipe_border(y, inputp->position + inputp->pipewidth + 1, PIPEHORI, inplvl);
    }

    // first layer turn border
    for (int i = 0; i < PIPEHOLE_END_WIDTH && (inputp->position - PIPEHOLE_END_WIDTH + i) < MAPSIZEX; i++)
        addch_pipe_border(inputp->upheight - 1, inputp->position - PIPEHOLE_END_WIDTH + i, PIPEVERT, inplvl);
    for (int i = 0; i < PIPEHOLE_END_WIDTH && (inputp->position + 1 + inputp->pipewidth + 1 + i) < MAPSIZEX; i++)
        addch_pipe_border(inputp->upheight - 1, inputp->position + 1 + inputp->pipewidth + 1 + i, PIPEVERT, inplvl);

    // second layer turn border and body
    for (int i = 0; i < inputp->pipewidth + 2 + PIPEHOLE_END_WIDTH * 2 && (inputp->position - 2 + i) < MAPSIZEX; i++)
    {
        if (i == 0 || i == inputp->pipewidth + 1 + PIPEHOLE_END_WIDTH * 2)
            addch_pipe_border(inputp->upheight, inputp->position - 2 + i, PIPEHORI, inplvl);
        else
            addch_pipe_body(inputp->upheight, inputp->position - 2 + i, PIPEBODY, inplvl);

        // third/last layer turn border
        addch_pipe_border(inputp->upheight + 1, inputp->position - 2 + i, PIPEEND, inplvl);
    }

    // RENDER DOWN PIPE
    for (int y = 0; y < inputp->downheight && inputp->position < MAPSIZEX; y++)
    {
        addch_pipe_border(MAPSIZEY - 1 - y, inputp->position, PIPEHORI, inplvl);
        for (int x = 0; x < inputp->pipewidth && inputp->position + x + 1 < MAPSIZEX; x++)
            addch_pipe_body(MAPSIZEY - 1 - y, inputp->position + x + 1, PIPEBODY, inplvl);
        if ((inputp->position + inputp->pipewidth + 1) < MAPSIZEX)
            addch_pipe_border(MAPSIZEY - 1 - y, inputp->position + inputp->pipewidth + 1, PIPEHORI, inplvl);
    }

    // first layer turn border
    for (int i = 0; i < PIPEHOLE_END_WIDTH && (inputp->position - PIPEHOLE_END_WIDTH + i) < MAPSIZEX; i++)
        addch_pipe_border(MAPSIZEY - 1 - inputp->downheight + 1, inputp->position - PIPEHOLE_END_WIDTH + i, '-', inplvl);
    for (int i = 0; i < PIPEHOLE_END_WIDTH && (inputp->position + 1 + inputp->pipewidth + 1 + i) < MAPSIZEX; i++)
        addch_pipe_border(MAPSIZEY - 1 - inputp->downheight + 1, inputp->position + 1 + inputp->pipewidth + 1 + i, '-', inplvl);

    // second layer turn border and body
    for (int i = 0; i < inputp->pipewidth + 2 + PIPEHOLE_END_WIDTH * 2 && (inputp->position - 2 + i) < MAPSIZEX; i++)
    {
        if (i == 0 || i == inputp->pipewidth + 1 + PIPEHOLE_END_WIDTH * 2)
            addch_pipe_border(MAPSIZEY - 1 - inputp->downheight, inputp->position - 2 + i, PIPEHORI, inplvl);
        else
            addch_pipe_body(MAPSIZEY - 1 - inputp->downheight, inputp->position - 2 + i, PIPEBODY, inplvl);

        // third/last layer turn border
        addch_pipe_border(MAPSIZEY - 1 - inputp->downheight - 1, inputp->position - 2 + i, PIPEEND, inplvl);
    }

    unsetcolor_bits(inplvl->pipe_color_brd, bitscolor_bg_to_fg(inplvl->pipe_color_brd));
    unsetcolor_bits(inplvl->pipe_color_body, bitscolor_bg_to_fg(inplvl->pipe_color_body));

    return 0;
}

/// @brief Will render pipes
/// @param inplvl Pointer to input level
/// @return Errcode
int render_pipes(level *inplvl)
{
    if (!inplvl)
        return -1;

    for (int i = 0; i < MAX_PIPES; i++)
        if (pipe_array[i].enabled)
            render_pipe(&pipe_array[i], inplvl);

    return 0;
}

/// @brief Will print level menu-options
/// @param option_selected Option that will be highlighted
/// @return Error code
int print_level_options(int option_selected)
{
    char level_options[6][30] = {
        "Play level",
        "Scroll down",
        "Scroll up",
        "Next level",
        "Previous level",
        "Back",
    };
    return print_header_options(6, 30, level_options, option_selected, act_screen.header_height - 1);
}

/// @brief Common function to print header options
/// @param option_items_n Count of items
/// @param maxstrlen Maximum one option string length
/// @param option_items Lines of strings (2D char array)
/// @param option_selected Which option will be highlighted
/// @param ypos Y position in header
/// @return
int print_header_options(int option_items_n, int maxstrlen, char option_items[option_items_n][maxstrlen], int option_selected, int ypos)
{
    setcolor_bits(act_screen.header_color, bitscolor_bg_to_fg(act_screen.header_color));
    clear_header(true);

    int actx = headeroffsx;
    int acty = headeroffsy + ypos;
    for (int i = 0; i < option_items_n; i++)
    {
        if (option_selected == i)
            setcolor_bits(bitscolor_bg_to_fg(act_screen.header_color), act_screen.header_color);

        mvprintw(acty, actx, option_items[i]);
        actx += strlen(option_items[i]) + 1;

        if (option_selected == i)
        {
            unsetcolor_bits(bitscolor_bg_to_fg(act_screen.header_color), act_screen.header_color);
            setcolor_bits(act_screen.header_color, bitscolor_bg_to_fg(act_screen.header_color));
        }

        if (i + 1 != option_items_n)
            mvprintw(acty, actx, "|");
        actx += 2;
    }

    unsetcolor_bits(act_screen.header_color, bitscolor_bg_to_fg(act_screen.header_color));
    return 0;
}

/// @brief Print level info in map area
/// @param inplvl Level struct pointer to use
/// @param yoffset Actual scroll
/// @return Error code
int print_level_info(level *inplvl, int yoffset)
{
    if (!inplvl)
        return -1;

    char lvlinfo[30][64] = {0};

    int lineidx = 0;
    sprintf(lvlinfo[lineidx++], "Level no.: %d", inplvl->levelnumber);

    if (strlen(inplvl->levelname) > 0)
        sprintf(lvlinfo[lineidx++], "Level name: %s", inplvl->levelname);

    sprintf(lvlinfo[lineidx++], "Start speed: %.3f [m/s]", inplvl->start_speed);

    if (inplvl->speed_increase > 0)
        sprintf(lvlinfo[lineidx++], "Speed increase: %.3f [m/s per minute]", inplvl->start_speed);
    else
        sprintf(lvlinfo[lineidx++], "Speed increase: %.3f [m/s per minute] (DEFAULT)", def_speed_incr);

    sprintf(lvlinfo[lineidx++], "Minimum space between pipes: %d [characters]", inplvl->minimum_space);
    sprintf(lvlinfo[lineidx++], "Maximum space between pipes: %d [characters]", inplvl->maximum_space);
    sprintf(lvlinfo[lineidx++], "Minimum width between pipes: %d [characters]", inplvl->minimum_width);
    sprintf(lvlinfo[lineidx++], "Maximum width between pipes: %d [characters]", inplvl->maximum_width);
    sprintf(lvlinfo[lineidx++], "Minimum distance between pipes: %d [characters]", inplvl->minimum_distance);
    sprintf(lvlinfo[lineidx++], "Maximum distance between pipes: %d [characters]", inplvl->maximum_distance);
    sprintf(lvlinfo[lineidx++], "Minimum distance between pipe spaces: %d [characters]", inplvl->minimum_distance_space);
    sprintf(lvlinfo[lineidx++], "Maximum distance between pipe spaces: %d [characters]", inplvl->maximum_distance_space);

    if (inplvl->gravity_multiply > 0)
        sprintf(lvlinfo[lineidx++], "Gravity multiplier: %.3f", inplvl->gravity_multiply);
    else
    {
        inplvl->gravity_multiply = def_grav_multiply;
        sprintf(lvlinfo[lineidx++], "Gravity multiplier: %.3f (DEFAULT)", def_grav_multiply);
    }

    sprintf(lvlinfo[lineidx++], "Gravity %.3f [m/s^2]", gravity_constant * inplvl->gravity_multiply);
    sprintf(lvlinfo[lineidx++], "Jump speed: %.3f [m/s]", inplvl->jump_speed);
    sprintf(lvlinfo[lineidx++], "Max lives: %d", inplvl->max_lives);

    int bgoppcolor = native_to_bitscolor(opposit_col(bits_to_native_color(bitscolor_bg_to_fg(inplvl->bgcolor))), true);
    setcolor_bits(bgoppcolor, bitscolor_bg_to_fg(inplvl->bgcolor));
    clear_map_area(NULL, true);

    fbpipe tmpp = get_pipe(4 * (MAPSIZEX / 5), inplvl, false, -1);

    render_pipe(&tmpp, inplvl);

    int lineidxcounter = yoffset, i = 0;
    for (; lineidxcounter < lineidx && i < MAPSIZEY - 1; i = i + 2)
    {
        mvprintw(mapoffsy + i, mapoffsx, lvlinfo[lineidxcounter]);
        lineidxcounter++;
    }
    if (i >= MAPSIZEY - 1 && lineidxcounter < lineidx)
        mvprintw(mapoffsy + i - 1, mapoffsx, "........");

    unsetcolor_bits(bgoppcolor, bitscolor_bg_to_fg(inplvl->bgcolor));

    if (i >= MAPSIZEY - 1 && lineidxcounter < lineidx)
        return ((lineidx * 2) - 1) - MAPSIZEY;

    return 0;
}

/// @brief Support function to render bird floating based on actual degree
/// @param inplvl Level struct pointer to use
/// @param degrees Degress to use to compute
/// @return Error code
int render_bird_floating(level *inplvl, int degrees)
{
    if (!inplvl)
        return -1;
    bird tmpb = get_bird(inplvl);
    double rad = ((2 * M_PI) / 360) * degrees;

    double beforerad = ((2 * M_PI) / 360) * (degrees - 2);
    int beforeposy = ((MAPSIZEY / 2) + (6 * sin(beforerad)));
    int beforeposx = (3 * (MAPSIZEX / 5)) - 2;
    if (beforeposy > 0)
        beforeposy--;

    setcolor_bits(inplvl->bgcolor, bitscolor_bg_to_fg(inplvl->bgcolor));
    for (int x = 0; x < 6; x++)
        for (int y = 0; y < 3; y++)
            mvaddch(mapoffsy + beforeposy + y, mapoffsx + beforeposx + x, ' ');
    unsetcolor_bits(inplvl->bgcolor, bitscolor_bg_to_fg(inplvl->bgcolor));

    tmpb.act_position = (MAPSIZEY / 2) + (6 * sin(rad));
    render_bird(&tmpb, 3 * (MAPSIZEX / 5), false);
    if (degrees + 2 >= 360)
        degrees = -2;

    refresh();
    msleep(1000 / act_rndsett.fps);

    return degrees + 2;
}

/// @brief Will render hall of fame
/// @param hoff Data pointer to use
/// @param yoff Scroll
/// @param dofree If do free on data unit
/// @param actnickname Nickname to highlight
/// @return If there is any scroll left
bool render_hof(config_option_t hoff, int yoff, bool dofree, char actnickname[64])
{
    config_option_t prev = NULL;
    int actoff = 0;
    bool skipalr = false;

    int bgoppcolor = native_to_bitscolor(opposit_col(bits_to_native_color(bitscolor_bg_to_fg(act_screen.header_color))), true);
    setcolor_bits(bgoppcolor, bitscolor_bg_to_fg(act_screen.header_color));
    clear_map_area(NULL, true);

    if (!hoff)
    {
        mvprintw(mapoffsy, mapoffsx, "Hall of fame is empty!");
        unsetcolor_bits(bgoppcolor, bitscolor_bg_to_fg(act_screen.header_color));

        return false;
    }
    if (hoff->key[0] == '\0')
    {
        mvprintw(mapoffsy, mapoffsx, "Hall of fame is empty!");
    }

    while (hoff)
    {
        if (hoff->key[0] != '\0')
        {
            if (actoff >= MAPSIZEY - 1)
                skipalr = true;

            if (actoff >= yoff && !skipalr)
            {
                char outtext[120] = {0};
                char *havelvl = strstr(hoff->key, "#lvl_");
                char nickname[64] = {0};
                memcpy(nickname, hoff->key, (size_t)(strstr(hoff->key, "#lvl_") - hoff->key) >= 64 ? 63 : (size_t)(strstr(hoff->key, "#lvl_") - hoff->key));
                int lvlout = -1;
                if (havelvl)
                {
                    lvlout = atoi(havelvl + 5);
                }

                sprintf(outtext, "%s - level: %d, max-score: %s", nickname, lvlout, hoff->value);

                if (strcmp(nickname, actnickname) == 0)
                    setcolor_bits(bitscolor_bg_to_fg(act_screen.header_color), bgoppcolor);
                mvprintw(mapoffsy + actoff, mapoffsx, "%s", outtext);
                if (strcmp(nickname, actnickname) == 0)
                    setcolor_bits(bgoppcolor, bitscolor_bg_to_fg(act_screen.header_color));

                actoff++;
            }
        }

        if (dofree)
        {
            prev = hoff->prev;
            free(hoff);
            hoff = prev;
        }
        else
            hoff = hoff->prev;
    }

    if (actoff >= MAPSIZEY - 1 && skipalr)
        mvprintw(mapoffsy + actoff, mapoffsx, "There is more! (use up and down arrows)");

    unsetcolor_bits(bgoppcolor, bitscolor_bg_to_fg(act_screen.header_color));

    return skipalr;
}

/// @brief Generate bird based on level
/// @param inplvl Level struct pointer to use
/// @return Generated bird structure
bird get_bird(level *inplvl)
{
    bird tmpbird = {0};
    if (!inplvl)
        return tmpbird;

    tmpbird.act_speed = 0;
    tmpbird.act_position = (int)(MAPSIZEY / 2);
    // Set color of bird that is exact oposite of background color, to be more seen

    tmpbird.colorbits = native_to_bitscolor(opposit_col(bits_to_native_color(bitscolor_bg_to_fg(inplvl->bgcolor))), true) | (inplvl->bgcolor & (7 << 4));
    tmpbird.gravity = gravity_constant * inplvl->gravity_multiply;
    tmpbird.jump_speed = inplvl->jump_speed;
    tmpbird.last_time_ms = timeInMilliseconds();

    return tmpbird;
}

/// @brief Function is checking collision between input bird and array of pipes, run before bird render!
/// @param inbird Input bird pointer that needs to be checked
/// @param xpos x map offset for bird
/// @return true if there is collision
bool bird_collision(bird *inpb, int xpos)
{
    if (!inpb)
        return false;
    int xcenter = mapoffsx + xpos;
    int ycenter = mapoffsy + inpb->act_position;

    if (ycenter - 1 >= mapoffsy && ycenter - 1 < (mapoffsy + MAPSIZEY))
        if ((mvinch(ycenter - 1, xcenter) & 255) != ' ')
            return true;

    if (ycenter >= mapoffsy && ycenter < (mapoffsy + MAPSIZEY))
    {
        if ((mvinch(ycenter, xcenter - 2) & 255) != ' ')
            return true;
        if ((mvinch(ycenter, xcenter + 3) & 255) != ' ')
            return true;
        if ((mvinch(ycenter, xcenter + 2) & 255) != ' ')
            return true;

        for (int i = 0; i < 3; i++)
            if ((mvinch(ycenter, xcenter - 1 + i) & 255) != ' ')
                return true;
    }

    if (ycenter + 1 >= mapoffsy && ycenter + 1 < (mapoffsy + MAPSIZEY))
        if ((mvinch(ycenter + 1, xcenter) & 255) != ' ')
            return true;

    return false;
}

/// @brief Will get most away pipe (with biggest x coordinate)
/// @return Pointer to pipe
fbpipe *get_most_away_pipe()
{
    int xpos = -1;
    fbpipe *retpipe = NULL;
    bool first = true;
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (!pipe_array[i].enabled)
            continue;

        if (first)
        {
            xpos = pipe_array[i].position;
            retpipe = &pipe_array[i];
            first = false;
        }
        else if (pipe_array[i].position > xpos)
        {
            retpipe = &pipe_array[i];
            xpos = pipe_array[i].position;
        }
    }
    return retpipe;
}

/// @brief Will get least away pipe (with lowest x coordinate)
/// @return Pointer to pipe
fbpipe *get_least_away_pipe()
{
    int xpos = -1;
    fbpipe *retpipe = NULL;
    bool first = true;
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (!pipe_array[i].enabled)
            continue;

        if (first)
        {
            xpos = pipe_array[i].position;
            retpipe = &pipe_array[i];
            first = false;
        }
        else if (pipe_array[i].position < xpos)
        {
            retpipe = &pipe_array[i];
            xpos = pipe_array[i].position;
        }
    }

    return retpipe;
}

/// @brief Will init speed based on level
/// @param inplvl level struct pointer to use
/// @return Error code
int init_speed(level *inplvl)
{
    if (!inplvl)
        return -1;
    act_speed_chars = METERTOCHARS * inplvl->start_speed;
    return 0;
}

/// @brief Increase speed based on last updated time variable
/// @param inplvl level struct pointer to use
/// @return Error code
int increase_speed(level *inplvl)
{
    if (!inplvl)
        return -1;
    if (last_time == 0)
    {
        last_time = timeInMilliseconds();
        return 0;
    }

    long long act_time = timeInMilliseconds();
    long long timediff = act_time - last_time;
    act_speed_chars += (float)(METERTOCHARS * (inplvl->speed_increase / (float)60)) * ((float)timediff / 1000);
    last_time = timeInMilliseconds();
    return 0;
}

/// @brief Get any free pipe slot
/// @return Pointer to free pipe slot
fbpipe *get_free_pipe_slot()
{
    for (int i = 0; i < MAX_PIPES; i++)
        if (!pipe_array[i].enabled)
            return &pipe_array[i];

    return NULL;
}

/// @brief Function to proccess/move pipes/world
/// @param inplvl
/// @return How many pipes has bird came accross
int move_pipes(level *inplvl)
{
    if (!inplvl)
        return -1;

    long long act_time = timeInMilliseconds();
    int counter = 0;

    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (pipe_array[i].last_time_moved <= 0)
        {
            pipe_array[i].last_time_moved = act_time;
            continue;
        }

        long long timediff = (act_time - pipe_array[i].last_time_moved);
        if (!pipe_array[i].enabled)
            continue;

        int posbef = pipe_array[i].position + 1 + pipe_array[i].pipewidth + PIPEHOLE_END_WIDTH;

        if ((act_speed_chars * ((float)timediff / (float)1000)) >= 1)
        {
            pipe_array[i].position -= (int)(act_speed_chars * ((float)timediff / (float)1000));
            pipe_array[i].last_time_moved = act_time;
        }

        int posaf = pipe_array[i].position + 1 + pipe_array[i].pipewidth + PIPEHOLE_END_WIDTH;
        if (posbef > BIRDOFFX && (posaf < BIRDOFFX || posaf == BIRDOFFX))
            counter++;
    }

    return counter;
}

/// @brief Process pipes (Generate new one if there is need)
/// @param inplvl level struct pointer to use
/// @return Error code
int process_pipes(level *inplvl)
{
    if (!inplvl)
        return -1;

    fbpipe *mostaway = get_most_away_pipe();
    fbpipe *leastaway = get_least_away_pipe();

    if (leastaway && leastaway->position + 1 + leastaway->pipewidth + PIPEHOLE_END_WIDTH < 0)
        leastaway->enabled = false;

    fbpipe *freepipe = get_free_pipe_slot();
    if (freepipe)
    {
        if (!mostaway)
            *freepipe = get_pipe(MAPSIZEX - 1 + PIPEHOLE_END_WIDTH, inplvl, true, -1);
        else if (mostaway->position + PIPEHOLE_END_WIDTH < MAPSIZEX)
            *freepipe = get_pipe(mostaway->position + mostaway->pipewidth + 1 + PIPEHOLE_END_WIDTH + rand_gen(inplvl->minimum_distance, inplvl->maximum_distance), inplvl, true, mostaway->upheight);
    }
    return 0;
}

/// @brief Will load level file based on level numner
/// @param levelnum Level numnber
/// @return Level struct
level load_level_file(int levelnum)
{
    char levelname[30] = {0};
    sprintf(levelname, "./assets/levels/level_%d.conf", levelnum);
    config_option_t options = read_config_file(levelname);
    level tmplevel = {0};
    tmplevel.levelnumber = levelnum;

    if (!options)
        return tmplevel;

    if (options->key[0] == '\0')
        return tmplevel;

    tmplevel.loaded = true;

    while (options != NULL)
    {
        if (strcmp(options->key, "bgcolor") == 0)
            tmplevel.bgcolor = native_to_bitscolor_bg(string_to_color(options->value));
        else if (strcmp(options->key, "pipecolor_border_fgcol") == 0)
            tmplevel.pipe_color_brd = tmplevel.pipe_color_brd | native_to_bitscolor(string_to_color(options->value), is_bold(options->value));
        else if (strcmp(options->key, "pipecolor_border_bgcol") == 0)
            tmplevel.pipe_color_brd = tmplevel.pipe_color_brd | native_to_bitscolor_bg(string_to_color(options->value));
        else if (strcmp(options->key, "pipecolor_body_fgcol") == 0)
            tmplevel.pipe_color_body = tmplevel.pipe_color_body | native_to_bitscolor(string_to_color(options->value), is_bold(options->value));
        else if (strcmp(options->key, "pipecolor_body_bgcol") == 0)
            tmplevel.pipe_color_body = tmplevel.pipe_color_body | native_to_bitscolor_bg(string_to_color(options->value));
        else if (strcmp(options->key, "start_speed") == 0)
            tmplevel.start_speed = atof(options->value);
        else if (strcmp(options->key, "speed_increase") == 0)
            tmplevel.speed_increase = atof(options->value);
        else if (strcmp(options->key, "minimum_space") == 0)
            tmplevel.minimum_space = atoi(options->value);
        else if (strcmp(options->key, "maximum_space") == 0)
            tmplevel.maximum_space = atoi(options->value);
        else if (strcmp(options->key, "minimum_width") == 0)
            tmplevel.minimum_width = atoi(options->value);
        else if (strcmp(options->key, "maximum_width") == 0)
            tmplevel.maximum_width = atoi(options->value);
        else if (strcmp(options->key, "minimum_distance") == 0)
            tmplevel.minimum_distance = atoi(options->value);
        else if (strcmp(options->key, "maximum_distance") == 0)
            tmplevel.maximum_distance = atoi(options->value);
        else if (strcmp(options->key, "minimum_distance_space") == 0)
            tmplevel.minimum_distance_space = atoi(options->value);
        else if (strcmp(options->key, "maximum_distance_space") == 0)
            tmplevel.maximum_distance_space = atoi(options->value);
        else if (strcmp(options->key, "gravity_multiply") == 0)
            tmplevel.gravity_multiply = atof(options->value);
        else if (strcmp(options->key, "jump_speed") == 0)
            tmplevel.jump_speed = atof(options->value);
        else if (strcmp(options->key, "max_lives") == 0)
            tmplevel.max_lives = atoi(options->value);
        else if (strcmp(options->key, "level_name") == 0)
            strcpy(tmplevel.levelname, options->value);

        config_option_t prev = options->prev;
        free(options);
        options = prev;
    }

    if (tmplevel.gravity_multiply == 0)
    {
        tmplevel.gravity_multiply = def_grav_multiply;
    }
    if (tmplevel.speed_increase == 0)
    {
        tmplevel.speed_increase = def_speed_incr;
    }

    return tmplevel;
}

/// @brief Load settings from file
/// @return Error code
int load_settings()
{
    config_option_t options = read_config_file(SETTINGS_FILE);
    while (options != NULL)
    {
        if (strcmp(options->key, "bordercolor_fg") == 0)
            act_screen.border_color = act_screen.border_color | native_to_bitscolor(string_to_color(options->value), is_bold(options->value));
        else if (strcmp(options->key, "bordercolor_bg") == 0)
            act_screen.border_color = act_screen.border_color | native_to_bitscolor_bg(string_to_color(options->value));
        else if (strcmp(options->key, "header_fg") == 0)
            act_screen.header_color = act_screen.header_color | native_to_bitscolor(string_to_color(options->value), is_bold(options->value));
        else if (strcmp(options->key, "header_bg") == 0)
            act_screen.header_color = act_screen.header_color | native_to_bitscolor_bg(string_to_color(options->value));
        else if (strcmp(options->key, "header_height") == 0)
            act_screen.header_height = atoi(options->value);
        else if (strcmp(options->key, "header_padding") == 0)
            act_screen.header_padding = atoi(options->value);
        else if (strcmp(options->key, "fps") == 0)
            act_rndsett.fps = atoi(options->value);
        else if (strcmp(options->key, "gravity_constant") == 0)
            gravity_constant = atof(options->value);
        else if (strcmp(options->key, "default_gravity_multiplier") == 0)
            def_grav_multiply = atof(options->value);
        else if (strcmp(options->key, "default_speed_increase_per_minute") == 0)
            def_speed_incr = atof(options->value);

        config_option_t prev = options->prev;
        free(options);
        options = prev;
    }
    return 0;
}
