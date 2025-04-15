#include "../include/game.h"
#include <ncurses.h>
#include <string.h>
#include <vector>
#include <string>
#include <algorithm>
#include <locale>

// Constants
const int MIN_HEIGHT = 25;
const int MIN_WIDTH = 80;

// Constructor initializes ncurses and create main window
Game::Game() : menuHighlight(0), menuItems{"New Game", "Load", "Exit"} {
    initscr();             // Initialize ncurses
    cbreak();              // Disable line buffering
    noecho();              // Don't show input
    curs_set(0);           // Don't show curses
    keypad(stdscr, TRUE);  // Enable special keys

    // Get screen size
    getmaxyx(stdscr, height, width);

    // Create the main window
    mainWindow = newwin(height, width, 0, 0);
    box(mainWindow, 0, 0);
    refresh();
    wrefresh(mainWindow);
}

// Destructor deletes game instance when exits main
Game::~Game() {
    delwin(mainWindow);
    endwin();
}

// Display Functions

void Game::displayInitialResizePrompt() {
    int currentHeight, currentWidth;
    int ch;

    // Ensure locale is set for std::to_string potentially
    std::setlocale(LC_ALL, "");

    nodelay(mainWindow, FALSE); // Ensure wgetch waits for input here

    do {
        getmaxyx(stdscr, currentHeight, currentWidth); // Get current terminal size

        // Update game's internal dimensions and ncurses window size if they changed
        // This allows the prompt itself to react to resizing
        if (currentHeight != height || currentWidth != width) {
             height = currentHeight;
             width = currentWidth;
             wresize(mainWindow, height, width);
             mvwin(mainWindow, 0, 0); // Ensure window is at top-left
        }

        werase(mainWindow); // Clear the window content
        box(mainWindow, 0, 0); // Redraw the border

        // Prepare messages
        std::string msg1 = "For the best experience, please resize the terminal to at least ";
        msg1 += std::to_string(MIN_WIDTH) + "x" + std::to_string(MIN_HEIGHT) + ".";
        std::string msg1_sub = "If you can't see the right boundary, please resize the terminal.";

        std::string msg2 = "Current size: " + std::to_string(currentWidth) + "x" + std::to_string(currentHeight);
        std::string msg3 = "Press ENTER to continue...";

        // Calculate centered positions using potentially updated dimensions
        int row1 = height / 2 - 3;
        int row1_sub = height / 2 - 2;
        int row2 = height / 2;
        int row3 = height / 2 + 2;

        int col1 = std::max(1, (width - (int)msg1.length()) / 2);
        int col1_sub = std::max(1, (width - (int)msg1_sub.length()) / 2);
        int col2 = std::max(1, (width - (int)msg2.length()) / 2);
        int col3 = std::max(1, (width - (int)msg3.length()) / 2);

        // Display messages - Check if dimensions are large enough to display
        if (height > 7 && width > (int)msg1.length() && width > (int)msg1_sub.length() && width > (int)msg2.length() && width > (int)msg3.length()) {
            wattron(mainWindow, COLOR_PAIR(1)); // Use a suitable color
            mvwprintw(mainWindow, row1, col1, "%s", msg1.c_str());
            mvwprintw(mainWindow, row1_sub, col1_sub, "%s", msg1_sub.c_str());
            wattroff(mainWindow, COLOR_PAIR(1));

            wattron(mainWindow, COLOR_PAIR(2)); // Use a suitable color
            mvwprintw(mainWindow, row2, col2, "%s", msg2.c_str());
            mvwprintw(mainWindow, row3, col3, "%s", msg3.c_str());
            wattroff(mainWindow, COLOR_PAIR(2));
        } else {
             // Fallback if window is too small to even display the message properly
             const char* small_msg = "Terminal too small. Resize needed.";
             mvwprintw(mainWindow, height / 2, std::max(1, (width - (int)strlen(small_msg)) / 2), "%s", small_msg);
        }

        wrefresh(mainWindow); // Refresh the window to show changes

        // Wait for input
        ch = wgetch(mainWindow); // Use mainWindow's keypad setting

        // Explicitly handle resize event if received while waiting
        if (ch == KEY_RESIZE) {
            // The loop will automatically fetch new dimensions and redraw
            continue;
        }

    // Loop until Enter key is pressed
    } while (ch != '\n' && ch != KEY_ENTER);

    // Clear the prompt screen before proceeding
    werase(mainWindow);
    box(mainWindow, 0, 0);
    wrefresh(mainWindow);
}

void Game::displayMenu() {
    werase(mainWindow);
    box(mainWindow, 0, 0);

    for (size_t i = 0; i < menuItems.size(); ++i) {
        if (i == menuHighlight) {
            wattron(mainWindow, A_REVERSE);
        }
        mvwprintw(mainWindow, height / 2 - menuItems.size() / 2 + i,
                  width / 2 - menuItems[i].length() / 2, menuItems[i].c_str());
        wattroff(mainWindow, A_REVERSE);
    }

    wrefresh(mainWindow);
}

void Game::displayContent(const std::string& text) {
    werase(mainWindow);
    box(mainWindow, 0, 0);
    mvwprintw(mainWindow, height / 2, width / 2 - text.length() / 2, text.c_str());
    wgetch(mainWindow);
}

void Game::display_size_warning() {
    werase(mainWindow);

    attron(A_BOLD);
    box(mainWindow, 0, 0);
    std::string warning = "WARNING: Terminal size too small!";
    mvwprintw(mainWindow, height / 2, width / 2 - warning.length() / 2, warning.c_str());
    attroff(A_BOLD);
    wrefresh(mainWindow);
}

bool Game::checkSize() {
    int newHeight, newWidth;
    getmaxyx(stdscr, newHeight, newWidth);

    if (newHeight < MIN_HEIGHT || newWidth < MIN_WIDTH) {
        return false; // Size is too small = return false
    }

    // Add static variable to track first run
    static bool firstRun = true;
    
    // When the window is resized, check if the new size is different from the old size
    if (newHeight != height || newWidth != width || firstRun) {
        height = newHeight;
        width = newWidth;

        // Resize the ncurses window
        firstRun = false;

        // Adjust the main window size and position
        wresize(mainWindow, height, width);
        mvwin(mainWindow, 0, 0); // Ensure window is at top-left after resize

        // Re-draw necessary elements after resize
        // Mark the window and its background for complete redraw
        touchwin(stdscr);
        touchwin(mainWindow);
        wnoutrefresh(stdscr);
        werase(mainWindow);
        box(mainWindow, 0, 0); // Redraw box
        wnoutrefresh(mainWindow);
        doupdate();
    }
    return true; // Size is okay = return true
}

void Game::waitForResize() {
    display_size_warning(); // Show warning on stdscr
    
    nodelay(stdscr, TRUE); // Set non-blocking mode for stdscr
    
    bool correctSize = false;
    while (!correctSize) {
        int ch = getch();
        
        // Check for resize or error
        if (ch == KEY_RESIZE || ch == ERR) {
            int newHeight, newWidth;
            getmaxyx(stdscr, newHeight, newWidth);
            
            if (newHeight >= MIN_HEIGHT && newWidth >= MIN_WIDTH) {
                height = newHeight;
                width = newWidth;
                correctSize = true;
                break;
            } else {
                display_size_warning();
            }
        }
        
        napms(100);
    }
    
    nodelay(stdscr, FALSE);
    
    clear();
    refresh();
    
    wresize(mainWindow, height, width);
    mvwin(mainWindow, 0, 0);
    
    werase(mainWindow);
    box(mainWindow, 0, 0);
    
    // Back to the current game state
    switch (current_state) {
        case GameState::MAIN_MENU:
            displayMenu();
            break;
        case GameState::DIFFICULTY_SELECT:
            displayDifficultyMenu();
            break;
        default:
            wrefresh(mainWindow);
            break;
    }
}

void Game::newGame() {
    displayContent("Starting a new game. Press any key to return to menu.");
    // Gameplay logic all handled by startGame class
}

void Game::load() {
    displayContent("Loading game. Press any key to return to menu.");
    // Load game logic handled by load class
}

void Game::run() {
    int choice;
    
    displayInitialResizePrompt();

    // Trigger the initial size check
    // This is a workaround for the first run, as sometimes the window size is not set correctly
    clear();
    refresh();
    wresize(mainWindow, height, width);
    mvwin(mainWindow, 0, 0);
    werase(mainWindow);
    box(mainWindow, 0, 0);
    wrefresh(mainWindow);

    while (true) {
        if (!checkSize()) {
            waitForResize();
        }

        displayMenu();
        choice = getch();

        switch (choice) {
            case KEY_UP:
                menuHighlight = (menuHighlight > 0) ? menuHighlight - 1 : menuItems.size() - 1;
                break;
            case KEY_DOWN:
                menuHighlight = (menuHighlight < menuItems.size() - 1) ? menuHighlight + 1 : 0;
                break;
            case '\n':
                switch (menuHighlight) {
                    case 0:
                        newGame();
                        break;
                    case 1:
                        load();
                        break;
                    case 2:
                        return;
                }
                break;
        }
    }
}