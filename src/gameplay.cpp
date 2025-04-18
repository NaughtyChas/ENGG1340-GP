#include "../include/game.h"
#include "../include/gameplay.h"

#include <ncurses.h>
#include <string>
#include <vector> // Not used for now, but might be needed later
#include <cmath>

// Constructor initializes windows based on difficulty
Gameplay::Gameplay(const int &difficultyHighlight, GameState &current_state)
    : difficultyHighlight(difficultyHighlight),
      current_state(current_state),
      map_size(0),
      num_obs(0),
      num_pkg(0),
      currentStamina(200),
      maxStamina(200)
{
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
    mapWin = newwin(1, 1, 0, 0); // Dummy sizes
    statsWin = newwin(1, 1, 0, 0);
    timeWin = newwin(1, 1, 0, 0);
    legendWin = newwin(1, 1, 0, 0);
    staminaWin = newwin(3, 1, 0, 0);

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
    delwin(staminaWin);
}

void Gameplay::resizeWindows() {
    getmaxyx(stdscr, height, width);

    // Recalculate window positions and sizes
    // --- Map Window ---
    int mapHeight = map_size;
    int mapWidth = map_size * 2;
    int mapY = std::max(0, (height - mapHeight) / 2);
    int mapX = std::max(0, (width - mapWidth) / 2);

    // --- Side Panel Widths ---
    int legendWidth = width / 4;
    int timeWidth = width / 4;
    int statsWidth = width / 4;

    // --- Side Panel Heights & Positions ---
    int legendHeight = std::min(height, height / 2);
    int timeHeight = std::min(height, height / 4);
    int statsHeight = std::max(1, height - timeHeight);
    int legendX = 0;
    int legendY = 0;
    int timeX = std::max(0, width - timeWidth);
    int timeY = 0;
    int statsX = std::max(0, width - statsWidth);
    int statsY = std::min(height - 1, timeHeight);

    // --- Stamina Window ---
    int staminaHeight = 3; // We will use fixed height for the bar
    int staminaWidth = std::max(20, width / 2);
    staminaWidth = std::min(width, staminaWidth);
    int staminaY = height - staminaHeight;
    int staminaX = std::max(0, (width - staminaWidth) / 2);

    // --- Resize and Reposition ---
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
    wresize(staminaWin, staminaHeight, staminaWidth);
    mvwin(staminaWin, staminaY, staminaX);
}

void Gameplay::run() {
    // Clear remnants from mainWindow
    clear();
    refresh();

    // Initialize colors if not done elsewhere (ensure start_color() was called)
    if (has_colors()) {
         start_color();
         init_pair(1, COLOR_YELLOW, COLOR_BLACK);
         init_pair(2, COLOR_CYAN, COLOR_BLACK);
    }


    while(current_state != GameState::MAIN_MENU) {
        getmaxyx(stdscr, height, width);

        // Stage changes done to windows
        resizeWindows();
        displayMap();
        displayStats();
        displayTime();
        displayLegend();
        displayStaminaBar();

        // Update all windows at once
        doupdate();

        int ch = getch();

        // --- Basic Input Handling ---
        // Not that implemented yet.
        bool staminaChanged = false;
        switch(ch) {
            case 27:
                // TODO: might draw a new window to confirm exit
                current_state = GameState::MAIN_MENU;
                return;
            case KEY_RESIZE:
                clear();
                refresh();
                break;
            case 'a': // in case where the stamina is decreased
                 if (currentStamina > 0) {
                     currentStamina -= 10; // Decrease by 10 for now, press a for testing
                     currentStamina = std::max(0, currentStamina);
                     staminaChanged = true;
                 }
                 break;
             case 'd': // Increase stamina
                 if (currentStamina < maxStamina) {
                     currentStamina += 10; // Increase by 10 for now, also for testing
                     currentStamina = std::min(maxStamina, currentStamina);
                     staminaChanged = true;
                 }
                 break;
            case ERR: // No input
                break;
            // Other game input handling should be implemented here...
            // W, A, S, D for movement, Q for package handling, etc.
        }

        // --- Game Logic Update ---
        // Wrote these comments to remind you to add game logic updates here.
        // For example, if stamina changed, you might want to check if the player can move or perform actions.
        // If stamina changed, the next loop iteration will redraw the bar.
        // Add other game logic updates here (e.g., move player, check collisions)


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
    wattroff(legendWin, A_BOLD);

    // --- Starting row for content ---
    int row = 2;
    int col = 2;

    // --- Legend Content ---
    mvwprintw(legendWin, row++, col, "--- Legend ---");
    mvwprintw(legendWin, row++, col, " @: Player");
    mvwprintw(legendWin, row++, col, " #: Obstacle");
    mvwprintw(legendWin, row++, col, " ~: Speed Bump");
    mvwprintw(legendWin, row++, col, " $: Supply");
    mvwprintw(legendWin, row++, col, " O: Package");
    mvwprintw(legendWin, row++, col, " Q: Exit");
    row++;

    // --- Movement Controls ---
    mvwprintw(legendWin, row++, col, "--- Movement ---");
    mvwprintw(legendWin, row++, col, "   W: Move Up");
    mvwprintw(legendWin, row++, col, "   S: Move Down");
    mvwprintw(legendWin, row++, col, "   A: Move Left");
    mvwprintw(legendWin, row++, col, "   D: Move Right");
    row++;


    // --- Package Controls ---
    mvwprintw(legendWin, row++, col, "--- Package ---");
    mvwprintw(legendWin, row++, col, "   Q: Pick Up");
    mvwprintw(legendWin, row++, col, "   E: Drop current package");
    mvwprintw(legendWin, row++, col, " 1-5: Switch current package"); // Simplified
    row++;

    // --- Game Controls ---
    mvwprintw(legendWin, row++, col, "---- Game ----");
    mvwprintw(legendWin, row++, col, " ESC: Pause");
    mvwprintw(legendWin, row++, col, " Hold ESC: Quit");

    wnoutrefresh(legendWin);
}

void Gameplay::displayStaminaBar() {
    werase(staminaWin);
    box(staminaWin, 0, 0);

    // --- Title ---
    const char* title = " Stamina ";
    mvwprintw(staminaWin, 0, 2, title);

    // --- Bar Calculation ---
    int barWidth = getmaxx(staminaWin) - 4;
    int numFilled = 0;
    if (maxStamina > 0) {
         if (barWidth > 0) {
            numFilled = static_cast<int>(std::floor(static_cast<double>(currentStamina) / maxStamina * barWidth));
            numFilled = std::max(0, std::min(barWidth, numFilled));
         }
    } else {
         numFilled = 0;
    }

    // --- Draw Bar ---
    wmove(staminaWin, 1, 2);
    wattron(staminaWin, COLOR_PAIR(1));
    for (int i = 0; i < numFilled; ++i) {
        waddch(staminaWin, ACS_BLOCK); // ▓
    }
    wattroff(staminaWin, COLOR_PAIR(1));

    // --- Numerical Display ---
    std::string staminaText = std::to_string(currentStamina) + " / " + std::to_string(maxStamina);
    int textX = getmaxx(staminaWin) - 2 - staminaText.length();
    textX = std::max(2, textX); // Ensure it doesn't overwrite left border
    mvwprintw(staminaWin, 1, textX, staminaText.c_str());


    wnoutrefresh(staminaWin);
}
