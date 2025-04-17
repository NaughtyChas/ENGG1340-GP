#include "../include/game.h"
#include "../include/gameplay.h"

#include <ncurses.h>

// Constructor initializes windows based on difficulty
Gameplay::Gameplay(const int &difficultyHighlight, GameState &current_state)
    : difficultyHighlight(difficultyHighlight),
        current_state(current_state),
        map_size(0),
        num_obs(0),
        num_pkg(0) {
        // gameplay related variables are not yet decided
        switch (difficultyHighlight) {
            case 0:
                diff_str = "Easy";
                map_size = 15;
                num_obs = 5;
                num_pkg = 5;
                break;
            case 1:
                diff_str = "Medium";
                map_size = 20;
                num_obs = 6;
                num_pkg = 6;
                break;
            case 2:
                diff_str = "Hard";
                map_size = 25;
                num_obs = 7;
                num_pkg = 7;
                break;
            default:
                diff_str = "Unknown";
                break;
        }

        nodelay(stdscr, TRUE);
        getmaxyx(stdscr, height, width);

        // Initialize windows
        int mapHeight = map_size;
        int mapWidth = map_size * 2;
        int statsHeight = height * 4 / 3;
        int statsWidth = width / 4;
        int timeHeight = height / 4;
        int timeWidth = width / 4;
        int legendHeight = height;
        int legendWidth = width / 4;
        
        mapWin = newwin(mapHeight, mapWidth, height - mapHeight, (width - mapWidth) / 2);
        statsWin = newwin(statsHeight, statsWidth, timeHeight, width - statsWidth);
        timeWin = newwin(timeHeight, timeWidth, 0, width - timeWidth);
        legendWin = newwin(legendHeight, legendWidth, 0, 0);

        keypad(stdscr, TRUE);
    }

Gameplay::~Gameplay() {
    // Restore blocking input
    nodelay(stdscr, FALSE);

    // Delete all windows
    delwin(mapWin);
    delwin(statsWin);
    delwin(timeWin);
    delwin(legendWin);
}

void Gameplay::resizeWindows() {
    getmaxyx(stdscr, height, width);

    // Recalculate window positions and sizes
    int mapHeight = map_size;
    int mapWidth = map_size * 2;
    int statsHeight = height * 4 / 3;
    int statsWidth = width / 4;
    int timeHeight = height / 4;
    int timeWidth = width / 4;
    int legendHeight = height;
    int legendWidth = width / 4;

    // Resize and reposition the windows
    wresize(mapWin, mapHeight, mapWidth);
    mvwin(mapWin, height - mapHeight, (width - mapWidth) / 2);
    
    wresize(statsWin, statsHeight, statsWidth);
    mvwin(statsWin, timeHeight, width - statsWidth);
    
    wresize(timeWin, timeHeight, legendWidth);
    mvwin(timeWin, 0, width - timeWidth);
    
    wresize(legendWin, legendHeight, legendWidth);
    mvwin(legendWin, 0, 0);
}

void Gameplay::run() {
    // Clear remnants from mainWindow
    clear();
    refresh();

    while(current_state != GameState::MAIN_MENU) {
        getmaxyx(stdscr, height, width);

        // Stage changes done to four windows
        resizeWindows();
        displayMap();
        displayStats();
        displayTime();
        displayLegend();

        // Update all windows at once
        doupdate();

        int ch = getch();

        switch(ch) {
            case 27:
                // TODO: might draw a new window to confirm exit
                current_state = GameState::MAIN_MENU;
                return;
            case KEY_RESIZE:
                clear();
                refresh();
                break;
            case ERR:
                break;
        }
        
        napms(100);
    }
}

// Display functions
void Gameplay::displayMap() {
    werase(mapWin);
    box(mapWin, 0, 0);
    mvwprintw(mapWin, 0, 2, diff_str.c_str());
    wnoutrefresh(mapWin);
}

void Gameplay::displayStats() {
    werase(statsWin);
    box(statsWin, 0, 0);
    mvwprintw(statsWin, 0, 2, " Stats ");
    wnoutrefresh(statsWin);
}

void Gameplay::displayTime() {
    werase(timeWin);
    box(timeWin, 0, 0);
    mvwprintw(timeWin, 0, 2, " Time ");
    wnoutrefresh(timeWin);
}

void Gameplay::displayLegend() {
    werase(legendWin);
    box(legendWin, 0, 0);
    mvwprintw(legendWin, 0, 2, " Legend ");
    wnoutrefresh(legendWin);
}
