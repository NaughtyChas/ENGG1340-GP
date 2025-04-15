#include "../include/game.h"
#include <ncurses.h>
#include <string.h>
#include <vector>
#include <string>
#include <algorithm>

// Constants
const int MIN_HEIGHT = 25;
const int MIN_WIDTH = 80;

// Constructor initializes ncurses and create main window
Game::Game() : menuHighlight(0), menuItems{"Start", "Stats", "Exit"}, current_state(GameState::MAIN_MENU), difficultyHighlight(0), difficultyItems{"Easy", "Medium", "Hard"} {
    initscr();             // Initialize ncurses
    cbreak();              // Disable line buffering
    noecho();              // Don't show input
    curs_set(0);           // Don't show curses
    start_color();         // Enable color usage
    keypad(stdscr, TRUE);  // Enable special keys for stdscr initially

    // Set timeout for ESC key
    ESCDELAY = 25;

    // Initialize color pairs
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);

    // Get initial screen size - run() will handle the check and wait if needed
    getmaxyx(stdscr, height, width);

    // Create the main window
    mainWindow = newwin(height, width, 0, 0);
    keypad(mainWindow, TRUE); // Enable special keys for the main window specifically
    box(mainWindow, 0, 0);
    refresh(); // Refresh stdscr
    wrefresh(mainWindow); // Refresh the window
}

// Destructor
Game::~Game() {
    delwin(mainWindow);
    endwin();
}

// Display Functions

void Game::displayMenu() {
    werase(mainWindow);
    box(mainWindow, 0, 0);

    // Show title in the center
    const char* title = "Rebirth: Me Delivering Keeta in Doomsday";
    int titleLen = strlen(title);
    wattron(mainWindow, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(mainWindow, 3, std::max(1, (width - titleLen) / 2), "%s", title); // Gets centered
    wattroff(mainWindow, COLOR_PAIR(1) | A_BOLD);

    // These lines calculate the X positions for the menu and description columns
    // The menu will be on the left, and the description on the right.
    int menuX = std::max(1, width * 3 / 10);
    int descX = std::max(menuX + 15, width * 6 / 10);

    // Menu items
    int startY = height / 2 - menuItems.size() / 2;
    for (int i = 0; i < menuItems.size(); ++i) {
        if (i == menuHighlight) {
            wattron(mainWindow, A_REVERSE);
        }
        mvwprintw(mainWindow, startY + i, menuX, "%s", menuItems[i].c_str());
        if (i == menuHighlight) {
            wattroff(mainWindow, A_REVERSE);
        }
    }

    // Instructions
    int rightY = startY;
    wattron(mainWindow, COLOR_PAIR(2)); // Change the color pair accordingly @Art&Design

    mvwprintw(mainWindow, rightY++, descX, "-------------------------");
    mvwprintw(mainWindow, rightY++, descX, "Welcome, Player!");
    mvwprintw(mainWindow, rightY++, descX, " ");
    mvwprintw(mainWindow, rightY++, descX, "Navigate: UP/DOWN Arrows");
    mvwprintw(mainWindow, rightY++, descX, "Select:   ENTER");
    mvwprintw(mainWindow, rightY++, descX, " ");
    mvwprintw(mainWindow, rightY++, descX, "Select 'Start' to begin");
    mvwprintw(mainWindow, rightY++, descX, "your perilous journey.");
    mvwprintw(mainWindow, rightY++, descX, "Select 'Stats' to view");
    mvwprintw(mainWindow, rightY++, descX, "your past performance.");
    mvwprintw(mainWindow, rightY++, descX, "-------------------------");
    wattroff(mainWindow, COLOR_PAIR(2));

    wrefresh(mainWindow);
}

void Game::displayDifficultyMenu() {
    werase(mainWindow);
    box(mainWindow, 0, 0);

    // Secondary Title
    const char* title = "Select Difficulty";
    int titleLen = strlen(title);
    wattron(mainWindow, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(mainWindow, 3, std::max(1, (width - titleLen) / 2), "%s", title);
    wattroff(mainWindow, COLOR_PAIR(1) | A_BOLD);

    // Column calculations same as above
    int menuX = std::max(1, width * 3 / 10);
    int descX = std::max(menuX + 15, width * 6 / 10);

    // Difficulty Options
    int startY = height / 2 - difficultyItems.size() / 2;
    for (int i = 0; i < difficultyItems.size(); ++i) {
        if (i == difficultyHighlight) {
            wattron(mainWindow, A_REVERSE);
        }
        mvwprintw(mainWindow, startY + i, menuX, "%s", difficultyItems[i].c_str());
        if (i == difficultyHighlight) {
            wattroff(mainWindow, A_REVERSE);
        }
    }

    // Diff Details
    int rightY = startY - 2;
    wattron(mainWindow, COLOR_PAIR(2));
    mvwprintw(mainWindow, rightY++, descX, "-------------------------");

    std::string map_size, packages, obstacles, desc;
    switch (difficultyHighlight) {
        // EZ
        case 0:
            map_size = "9x9"; packages = "3"; obstacles = "5";
            desc = "A gentler start.";
            break;
        // Medium
        case 1:
            map_size = "11x11"; packages = "4"; obstacles = "6";
            desc = "A standard challenge.";
            break;
        // Hard
        case 2:
            map_size = "13x13"; packages = "5"; obstacles = "7";
            desc = "For the seasoned courier.";
            break;
    }

    mvwprintw(mainWindow, rightY++, descX, "Difficulty: %s", difficultyItems[difficultyHighlight].c_str());
    mvwprintw(mainWindow, rightY++, descX, "Description: %s", desc.c_str());
    mvwprintw(mainWindow, rightY++, descX, " ");
    mvwprintw(mainWindow, rightY++, descX, "Map Size:    %s", map_size.c_str());
    mvwprintw(mainWindow, rightY++, descX, "Packages:    %s", packages.c_str());
    mvwprintw(mainWindow, rightY++, descX, "Obstacles:   %s", obstacles.c_str());
    mvwprintw(mainWindow, rightY++, descX, " ");
    mvwprintw(mainWindow, rightY++, descX, "Stamina: 200 start, +50/lvl");
    mvwprintw(mainWindow, rightY++, descX, "Score based on completion,");
    mvwprintw(mainWindow, rightY++, descX, "time, and packages.");
    mvwprintw(mainWindow, rightY++, descX, "-------------------------");
    wattroff(mainWindow, COLOR_PAIR(2));

    // Bottom Instructions
    const char* instructions = "UP/DOWN to change, ENTER to confirm, ESC to go back.";
    int instrX = std::max(1, (width - (int)strlen(instructions)) / 2); // Center these texts
    mvwprintw(mainWindow, height - 3, instrX, "%s", instructions);

    wrefresh(mainWindow);
}

void Game::displayContent(const std::string& text) {
     werase(mainWindow);
     box(mainWindow, 0, 0);
     mvwprintw(mainWindow, height / 2, (width - text.length()) / 2, "%s", text.c_str());
     wgetch(mainWindow);  // wgetch refreshes window by default
}

void Game::display_size_warning(int &newHeight, int &newWidth) {
    getmaxyx(stdscr, newHeight, newWidth);
    int screen_height = LINES;
    int screen_width = COLS;
    std::string resize_text = "Please resize to 25 by 80 Before Starting Game";
    std::string current_size_text = "Current height: ";
    std::string width_label = ", width: ";

    int resize_text_len = resize_text.length();
    int current_size_text_len = current_size_text.length();
    int width_label_len = width_label.length();

    int resize_row = screen_height / 2 - 2; // Position a bit above the center
    int current_size_row = screen_height / 2;
    int center_resize_col = (screen_width - resize_text_len) / 2;
    int center_current_size_col = (screen_width - (current_size_text_len + std::to_string(newHeight).length() + width_label_len + std::to_string(newWidth).length())) / 2;

    werase(mainWindow); // Clear the window content
    box(mainWindow, 0, 0); // Redraw the border

    wattron(mainWindow, COLOR_PAIR(2));
    mvwprintw(mainWindow, resize_row, center_resize_col, "%s", resize_text.c_str());
    wattroff(mainWindow, COLOR_PAIR(2));

    wattron(mainWindow, COLOR_PAIR(2));
    mvwprintw(mainWindow, current_size_row, center_current_size_col, "%s", current_size_text.c_str());
    wattroff(mainWindow, COLOR_PAIR(2));

    wattron(mainWindow, COLOR_PAIR(1));
    mvwprintw(mainWindow, current_size_row, center_current_size_col + current_size_text_len, "%d", newHeight);
    wattroff(mainWindow, COLOR_PAIR(1));

    wattron(mainWindow, COLOR_PAIR(2));
    mvwprintw(mainWindow, current_size_row, center_current_size_col + current_size_text_len + std::to_string(newHeight).length(), "%s", width_label.c_str());
    wattroff(mainWindow, COLOR_PAIR(2));

    wattron(mainWindow, COLOR_PAIR(1));
    mvwprintw(mainWindow, current_size_row, center_current_size_col + current_size_text_len + std::to_string(newHeight).length() + width_label_len, "%d", newWidth);
    wattroff(mainWindow, COLOR_PAIR(1));

    wattron(mainWindow, COLOR_PAIR(2));
    mvwprintw(mainWindow, current_size_row + 2, (screen_width - 23) / 2, "Press Enter to continue...");
    wattroff(mainWindow, COLOR_PAIR(2));

    wrefresh(mainWindow);

    napms(100); // Wait briefly to avoid busy-waiting
}

void Game::ask_for_resize() {
    int choice;
    int newHeight, newWidth;

    werase(mainWindow);
    box(mainWindow, 0, 0);
    display_size_warning(newHeight, newWidth);

    while ((choice = getch()) != '\n') {
        display_size_warning(newHeight, newWidth);
    }
}

void Game::newGame(int difficulty) {
    std::string diff_str;
    switch(difficulty) {
        case 0: diff_str = "Easy"; break;
        case 1: diff_str = "Medium"; break;
        case 2: diff_str = "Hard"; break;
        default: diff_str = "Unknown"; break;
    }
    displayContent("Starting a new game on " + diff_str + " difficulty... (Gameplay TBD)");
    // TODO: Initialize game state based on difficulty (map size, packages, etc.)
    // TODO: Enter the actual game loop here (or elsewhere, I might be reconstructing it soon)
    current_state = GameState::MAIN_MENU; // Return to main menu for now
}

void Game::displayStats() {
    displayContent("Displaying Stats... (Not implemented)");
    // TODO: Implement stats display logic
    // For now, just show a message and wait for input to return
}

// Main Game Loop

void Game::run() {
    int choice;

    ask_for_resize();

    while (current_state != GameState::EXITING) { // Loop until exit state
        // // Size Check
        // if (!checkSize()) {
        //     waitForResize();
        //     // After resize, checkSize() will have redrawn the window if necessary.
        //     // Continue ensures the correct state's display function is called immediately.
        //     continue;
        // }

        // Display based on State
        switch (current_state) {
            case GameState::MAIN_MENU:
                displayMenu();
                break;
            case GameState::DIFFICULTY_SELECT:
                displayDifficultyMenu();
                break;
            case GameState::STATS:
                displayStats();
                break;
            // Add cases for IN_GAME, GAME_OVER etc. later
            default:
                 // Shouldn't happen but just in case
                 displayContent("Error: Unknown game state!");
                 current_state = GameState::MAIN_MENU; // Recover to main menu
                 break;
        }

        // Input Handling - Only process input if not already handled by the display function (like displayStats)
        if (current_state != GameState::STATS) {
            choice = wgetch(mainWindow); // Get input

            // State-Specific Input Processing
            switch (current_state) {
                case GameState::MAIN_MENU:
                    handleMainMenuInput(choice);
                    break;
                case GameState::DIFFICULTY_SELECT:
                    handleDifficultyInput(choice);
                    break;
                // Add cases for other states
            }
        } else {
            // If stats were displayed, return to main menu
            current_state = GameState::MAIN_MENU;
        }
    }
}

// Input Handlers

void Game::handleMainMenuInput(int choice) {
    switch (choice) {
        case KEY_UP:
            menuHighlight = (menuHighlight - 1 + menuItems.size()) % menuItems.size();
            break;
        case KEY_DOWN:
            menuHighlight = (menuHighlight + 1) % menuItems.size();
            break;
        case '\n':
        case KEY_ENTER:
            if (menuHighlight == 0) { // "Start" selected
                current_state = GameState::DIFFICULTY_SELECT;
                difficultyHighlight = 0; // Reset difficulty selection highlight
            } else if (menuHighlight == 1) { // "Stats" selected
                current_state = GameState::STATS;
            } else if (menuHighlight == 2) { // "Exit" selected
                current_state = GameState::EXITING;
            }
            break;
        // KEY_RESIZE is handled globally in run()
        default:
            break;
    }
}

void Game::handleDifficultyInput(int choice) {
     switch (choice) {
        case KEY_UP:
            difficultyHighlight = (difficultyHighlight - 1 + difficultyItems.size()) % difficultyItems.size();
            break;
        case KEY_DOWN:
            difficultyHighlight = (difficultyHighlight + 1) % difficultyItems.size();
            break;
        case '\n':
        case KEY_ENTER:
            // Pass the selected difficulty index (0=Easy, 1=Medium, 2=Hard)
            newGame(difficultyHighlight);
            // newGame currently sets state back to MAIN_MENU.
            // Later, it will set state to IN_GAME.
            break;
        case 27: // ESC
        case KEY_BACKSPACE: // Often mapped similarly or preferred by users
             current_state = GameState::MAIN_MENU; // Go back to main menu
             menuHighlight = 0;
             break;
        // KEY_RESIZE is handled globally in run()
        default:
            break;
    }
}
