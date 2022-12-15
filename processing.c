#include "rendering.h"
#include "processing.h"
#include "common_tools.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define SAVES_FILE "./assets/saves.conf"
#define HALLOFFAME_FILE "./assets/hall_of_fame.conf"

char act_nick[64] = {0};
int get_last_level(char *nickname);
int set_last_level(char *nickname, int level);
int get_hall_of_fame(char nickname[], int level);
int set_hall_of_fame(char nickname[], int score, int level);
int process_run_level(level *inplvl);
int select_level_dialog();
void request_nickname();
int show_about_page();
int show_hof();

int process_option(int option)
{
    switch (option)
    {
    case 0:
    {
        int last_level = get_last_level(act_nick);

        if (last_level > 0)
        {
            level loadedlvl = load_level_file(last_level);
            if (!loadedlvl.loaded)
            {
                render_header_string("The level that is saved, does not exist! Setting to level 1.", 0, true, true);
                refresh();
                msleep(1500);
                flushinp();
                set_last_level(act_nick, 1);
            }
            else
                process_run_level(&loadedlvl);
        }
        else
        {
            render_header_string("You are playing this game for the first time, running level 1", 0, true, true);
            refresh();
            msleep(1500);
            flushinp();
            level loadedlvl = load_level_file(1);
            process_run_level(&loadedlvl);
        }
    }
    break;
    case 1:
    {
        int selectedlvl = select_level_dialog();
        if (selectedlvl > 0)
        {
            level loadedlvl = load_level_file(selectedlvl);
            process_run_level(&loadedlvl);
        }
    }
    break;
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

int select_level_dialog()
{
    int actoption = 0, actlvl = 1, scroll = 0, maxscroll = 0;
    keypad(stdscr, true);
    int degrees = 0;
    bool refrall = false;
    while (true)
    {
        print_level_options(actoption);
        level tmplvl = load_level_file(actlvl);
        maxscroll = print_level_info(&tmplvl, scroll);

        refresh();
        flushinp();
        timeout(0);
        refrall = false;

        while (true)
        {
            degrees = render_bird_floating(&tmplvl, degrees);
            switch (getch())
            {
            case KEY_RIGHT:
                if (actoption < 5)
                    actoption++;
                refrall = true;
                break;
            case KEY_LEFT:
                if (actoption > 0)
                    actoption--;
                refrall = true;
                break;
            case '\n':
                refrall = true;
                switch (actoption)
                {
                case 0:
                    return actlvl;
                case 1:
                    if (scroll < maxscroll)
                        scroll++;
                    break;
                case 2:
                    if (scroll > 0)
                        scroll--;
                    break;
                case 3:
                    if (load_level_file(actlvl + 1).loaded)
                        actlvl++;
                    break;
                case 4:
                    if (actlvl - 1 > 0 && load_level_file(actlvl - 1).loaded)
                        actlvl--;
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

            if (refrall)
                break;
        }
    }
}

int show_about_page()
{
    int scroll = 0, maxscroll = 0;
    timeout(-1);
    keypad(stdscr, true);
    render_header_string("This is \"about this game\" page, to exit, press 'b'", 1, true, true);
    while (true)
    {
        maxscroll = render_about_page(scroll);
        refresh();
        flushinp();
        int ch = getch();
        switch (tolower(ch))
        {
        case KEY_UP:
            if (scroll > 0)
                scroll--;
            break;
        case KEY_DOWN:
            if (scroll < maxscroll)
                scroll++;
            break;
        case 'b':
            timeout(0);
            keypad(stdscr, false);
            return 0;

        default:
            break;
        }
    }
}

int process_run_level(level *inplvl)
{
    if (!inplvl)
        return -1;

    if (!inplvl->loaded)
    {
        char outputstr[60] = {0};
        sprintf(outputstr, "The level %d does not exist! Exiting...", inplvl->levelnumber);
        render_header_string(outputstr, 0, true, true);
        refresh();
        msleep(1500);
        flushinp();
        return -1;
    }

    int status = 0;
    set_last_level(act_nick, inplvl->levelnumber);
    int score = run_level(inplvl, &status);

    int hof = get_hall_of_fame(act_nick, inplvl->levelnumber);
    char outstring[128] = {0};
    if (score > hof && score > 0)
    {
        sprintf(outstring, "GAME OVER! NEW HIGH SCORE FOR THIS LEVEL! %s - YOUR SCORE: %d [SAVING IT TO HALL OF FAME]", act_nick, score);
        set_hall_of_fame(act_nick, score, inplvl->levelnumber);
    }
    else
        sprintf(outstring, "GAME OVER! %s - YOUR SCORE: %d", act_nick, score);

    render_header_string(outstring, -1, true, true);
    refresh();

    msleep(1500);
    flushinp();
    return score;
}

int run_menu()
{
    int actoption = 0;
    while (true)
    {
        keypad(stdscr, true);
        render_menu(actoption, act_nick);
        refresh();
        flushinp();
        timeout(-1);
        int ch = getch();
        switch (ch)
        {
        case KEY_RIGHT:
            if (actoption < menu_inem_n - 1)
                actoption++;

            break;
        case KEY_LEFT:
            if (actoption > 0)
                actoption--;
            break;
        case '\n':
            if (actoption == 5)
            {
                keypad(stdscr, false);
                return 1;
            }

            process_option(actoption);
            break;

        default:
            break;
        }
    }
}

void request_nickname()
{
    while (true)
    {
        render_header_string("Please write here your nickname (max 63 chars): ", 1, true, true);
        refresh();
        echo();
        curs_set(1);
        flushinp();
        timeout(-1);
        turn_on_header_color(true);
        getstr(act_nick);
        turn_off_header_color(true);
        curs_set(0);
        noecho();
        if (strcmp("", act_nick) == 0)
        {
            render_header_string("Your nick cannot be empty!", 1, true, true);
            refresh();
            msleep(1500);
        }
        else
            break;
    }

    char outmessage[120] = {0};
    sprintf(outmessage, "Your nickname is now: %s", act_nick);
    render_header_string(outmessage, 1, true, true);
    refresh();

    msleep(1500);
}

int get_last_level(char nickname[])
{
    return get_int_key_value(SAVES_FILE, nickname);
}

int set_last_level(char nickname[], int inlevel)
{
    return set_int_key_value("./lvltmpfile", SAVES_FILE, nickname, inlevel);
}

int get_hall_of_fame(char nickname[], int level)
{
    char findkey[120] = {0};
    sprintf(findkey, "%s#lvl_%d#", nickname, level);
    return get_int_key_value(HALLOFFAME_FILE, findkey);
}

int set_hall_of_fame(char nickname[], int score, int level)
{
    char setkey[120] = {0};
    sprintf(setkey, "%s#lvl_%d#", nickname, level);
    return set_int_key_value("./hoftmpfile", HALLOFFAME_FILE, setkey, score);
}

int show_hof()
{
    int scroll = 0;
    bool scrollalowed = false;
    timeout(-1);
    keypad(stdscr, true);
    render_header_string("This is \"HALL OF FAME\" page, to exit, press 'b'", 1, true, true);
    config_option_t hof = read_config_file(HALLOFFAME_FILE);
    while (true)
    {
        scrollalowed = render_hof(hof, scroll, false, act_nick);
        refresh();
        flushinp();
        int ch = getch();
        switch (tolower(ch))
        {
        case KEY_UP:
            if (scroll > 0)
                scroll--;
            break;
        case KEY_DOWN:
            if (scrollalowed)
                scroll++;
            break;
        case 'b':
            timeout(0);
            keypad(stdscr, false);
            while (hof)
            {
                config_option_t prev = hof->prev;
                free(hof);
                hof = prev;
            }

            return 0;

        default:
            break;
        }
    }
}

int run_game()
{
    request_nickname();
    run_menu();
    return 0;
}