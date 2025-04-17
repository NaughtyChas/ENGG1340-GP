#include "../include/game.h"
#include "../include/gameplay.h"

#include <ncurses.h>
#include <string>

// Constructor initializes windows based on difficulty
Gameplay::Gameplay(const int &difficultyHighlight, GameState &current_state)
    : difficultyHighlight(difficultyHighlight),
      current_state(current_state),
      map_size(0),
      num_obs(0),
      num_pkg(0),
      roundNumber(1) { // Currently a placeholder. We will use this later on.
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
    // Adjusted initial sizes/positions
    int statsWidth = width / 4;
    int timeHeight = height / 4;
    int timeWidth = width / 4;
    int legendWidth = width / 4;
    int legendHeight = height / 2;
    int statsHeight = height - timeHeight;

    // Create windows
    mapWin = newwin(mapHeight, mapWidth, 1, 1);
    statsWin = newwin(statsHeight, statsWidth, 1, 1);
    timeWin = newwin(timeHeight, timeWidth, 1, 1);
    legendWin = newwin(legendHeight, legendWidth, 1, 1);

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

    // Map window has to be centered
    int mapY = (height - mapHeight) / 2;
    int mapX = (width - mapWidth) / 2;
    // Ensure positions are not negative if terminal is too small
    mapY = std::max(0, mapY);
    mapX = std::max(0, mapX);

    // Adjust other windows
    int legendWidth = width / 4;
    int timeWidth = width / 4;
    int statsWidth = width / 4;

    int legendHeight = height / 2;
    legendHeight = std::min(height, legendHeight);

    int timeHeight = height / 4;
    timeHeight = std::min(height, timeHeight);

    int statsHeight = height - timeHeight;
    statsHeight = std::max(1, statsHeight);

    int legendX = 0;
    int legendY = 0;

    int timeX = width - timeWidth;
    int timeY = 0;
    timeX = std::max(0, timeX);

    int statsX = width - statsWidth;
    int statsY = timeHeight;
    statsX = std::max(0, statsX);
    statsY = std::min(height - 1, statsY);

    // Resize and reposition the windows
    wresize(mapWin, mapHeight, mapWidth);
    mvwin(mapWin, mapY, mapX);

    wresize(legendWin, legendHeight, legendWidth);
    mvwin(legendWin, legendY, legendX);

    wresize(timeWin, timeHeight, timeWidth);
    mvwin(timeWin, timeY, timeX);

    wresize(statsWin, statsHeight, statsWidth);
    mvwin(statsWin, statsY, statsX);

    // Note: With the map centered, the side panels (Legend, Time, Stats)
    // might overlap the map if the terminal width is not large enough
    // maybe will carryout a check here to ensure that the map is not overlapped
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
    // Display Round Number
    std::string roundText = "Round " + std::to_string(roundNumber);
    wattron(legendWin, A_BOLD);
    mvwprintw(legendWin, 0, 2, " %s ", roundText.c_str());
    wattroff(legendWin, A_BOLD); // Tested on Windows build but the bold text is not correctly displayed
    // I have to commit this so I can test on Linux build in CS servers

    // Legend Content
    mvwprintw(legendWin, 2, 2, "Legend:");
    mvwprintw(legendWin, 3, 2, "@: Player");
    mvwprintw(legendWin, 4, 2, "#: Obstacle");
    mvwprintw(legendWin, 5, 2, "~: Speed Bump");
    mvwprintw(legendWin, 6, 2, "$: Supply");
    mvwprintw(legendWin, 7, 2, "O: Package");
    mvwprintw(legendWin, 8, 2, "Q: Exit");
    // I've randomly written some. Change them later on.

    wnoutrefresh(legendWin);
}
