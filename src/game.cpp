#include "../include/game.h"

#include <ncurses.h>
#include <string.h>
#include <fstream>
#include <algorithm>
#include <locale>
#include <string>
#include <vector>

#include "../include/gameplay.h"

// Include windows.h only on Windows platforms
// Make sure that the window will be maximized on Windows
#ifdef _WIN32
#include <windows.h>
#endif

// Constants
const int MIN_HEIGHT = 31;
const int MIN_WIDTH = 115;

// Constructor initializes ncurses and create main window
Game::Game()
    : menuHighlight(0),
      menuItems{"New Game", "Load Game", "Exit"},
      current_state(GameState::MAIN_MENU),
      difficultyHighlight(0),
      difficultyItems{"Easy", "Medium", "Hard"},
      isNewGame(false) {
#ifdef _WIN32
    // Get the console window handle and maximize it
    HWND consoleWindow = GetConsoleWindow();
    if (consoleWindow != NULL) {
        ShowWindow(consoleWindow, SW_MAXIMIZE);
    }
#endif

    initscr();             // Initialize ncurses AFTER maximizing attempt
    cbreak();              // Disable line buffering
    noecho();              // Don't show input
    curs_set(0);           // Don't show curses
    keypad(stdscr, TRUE);  // Enable special keys
    start_color();         // Enable color support
    set_escdelay(0);       // Remove ESC delay

    // Initialize color pairs
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);

    // Get initial screen size - run() will handle the check and wait if needed
    getmaxyx(stdscr, height, width);

    // Create the main window
    mainWindow = newwin(height, width, 0, 0);
    keypad(mainWindow, TRUE);  // Enable special keys for the main window specifically
    box(mainWindow, 0, 0);
    refresh();             // Refresh stdscr
    wrefresh(mainWindow);  // Refresh the window
}

// Destructor
Game::~Game() {
    // Check if save file exists and delete it
    if (std::remove("savegame.txt") == 0) {
        // File successfully deleted
    } else {
        // File might not exist
    }

    delwin(mainWindow);
    endwin();
}

// Display Functions

void Game::displayInitialResizePrompt() {
    int currentHeight, currentWidth;
    int ch;

    // Ensure locale is set for std::to_string potentially
    std::setlocale(LC_ALL, "");

    nodelay(mainWindow, FALSE);  // Ensure wgetch waits for input here

    do {
        getmaxyx(stdscr, currentHeight, currentWidth);  // Get current terminal size

        // Update game's internal dimensions and ncurses window size if they changed
        // This allows the prompt itself to react to resizing
        if (currentHeight != height || currentWidth != width) {
            height = currentHeight;
            width = currentWidth;
            wresize(mainWindow, height, width);
            mvwin(mainWindow, 0, 0);  // Ensure window is at top-left
        }

        werase(mainWindow);     // Clear the window content
        box(mainWindow, 0, 0);  // Redraw the border

        // Prepare messages
        std::string msg1 =
            "For best experience, please resize the terminal to FULL SCREEN, or at least ";
        msg1 += std::to_string(MIN_WIDTH) + "x" + std::to_string(MIN_HEIGHT) + ".";
        std::string msg1_sub = "If you can't see the right boundary, please resize the terminal.";

        std::string msg2 =
            "Current size: " + std::to_string(currentWidth) + "x" + std::to_string(currentHeight);
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
        if (height > 7 && width > (int)msg1.length() && width > (int)msg1_sub.length() &&
            width > (int)msg2.length() && width > (int)msg3.length()) {
            wattron(mainWindow, COLOR_PAIR(1));  // Use a suitable color
            mvwprintw(mainWindow, row1, col1, "%s", msg1.c_str());
            mvwprintw(mainWindow, row1_sub, col1_sub, "%s", msg1_sub.c_str());
            wattroff(mainWindow, COLOR_PAIR(1));

            wattron(mainWindow, COLOR_PAIR(2));  // Use a suitable color
            mvwprintw(mainWindow, row2, col2, "%s", msg2.c_str());
            mvwprintw(mainWindow, row3, col3, "%s", msg3.c_str());
            wattroff(mainWindow, COLOR_PAIR(2));
        } else {
            // Fallback if window is too small to even display the message properly
            const char* small_msg = "Terminal too small. Resize needed.";
            mvwprintw(mainWindow, height / 2, std::max(1, (width - (int)strlen(small_msg)) / 2),
                      "%s", small_msg);
        }

        // Wait for input
        ch = wgetch(mainWindow);  // Use mainWindow's keypad setting

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

    // ASCII Art Definitions
    // The whole ASCII Art is in its fine state so I suggest to keep it as is
    static const char* ascii_rebirth[] = {
        "     ____       __    _      __  __     ",    "    / __ \\___  / /_  (_)____/ /_/ /_  _ ",
        "   / /_/ / _ \\/ __ \\/ / ___/ __/ __ \\(_)", "  / _, _/  __/ /_/ / / /  / /_/ / / /   ",
        " /_/ |_|\\___/_.___/_/_/   \\__/_/ /_(_)  ",  "                                        "};
    static const char* ascii_subtitle[] = {
        "     __  ___        ____       ___                 _                __ __          __     "
        "      _          ____                                 __           ",
        "    /  |/  /__     / __ \\___  / (_)   _____  _____(_)___  ____ _   / //_/__  ___  / "
        "/_____ _   (_)___     / __ \\____  ____  ____ ___  _________/ /___ ___  __",
        "   / /|_/ / _ \\   / / / / _ \\/ / / | / / _ \\/ ___/ / __ \\/ __ `/  / ,< / _ \\/ _ \\/ "
        "__/ __ `/  / / __ \\   / / / / __ \\/ __ \\/ __ `__ \\/ ___/ __  / __ `/ / / /",
        "  / /  / /  __/  / /_/ /  __/ / /| |/ /  __/ /  / / / / / /_/ /  / /| /  __/  __/ /_/ /_/ "
        "/  / / / / /  / /_/ / /_/ / /_/ / / / / / (__  ) /_/ / /_/ / /_/ / ",
        " /_/  /_/\\___/  /_____/\\___/_/_/ |___/\\___/_/  /_/_/ /_/\\__, /  /_/ "
        "|_\\___/\\___/\\__/\\__,_/  /_/_/ /_/  /_____/\\____/\\____/_/ /_/ "
        "/_/____/\\__,_/\\__,_/\\__, /  ",
        "                                                       /____/                            "
        "                                                           /____/   "};
    int rebirth_lines = sizeof(ascii_rebirth) / sizeof(ascii_rebirth[0]);
    int subtitle_lines = sizeof(ascii_subtitle) / sizeof(ascii_subtitle[0]);
    int art_total_lines = rebirth_lines + subtitle_lines;
    int art_max_width = 0;  // Find the widest line for centering checks
    for (int i = 0; i < rebirth_lines; ++i)
        art_max_width = std::max(art_max_width, (int)strlen(ascii_rebirth[i]));
    for (int i = 0; i < subtitle_lines; ++i)
        art_max_width = std::max(art_max_width, (int)strlen(ascii_subtitle[i]));

    int art_start_y = 2;  // Start drawing art from row 2
    int menu_start_y;     // Where menu items will start vertically

    // Check if terminal is large enough for ASCII art
    if (width >= art_max_width + 2 &&
        height >= art_total_lines + 10) {  // +10 for padding and menu items
        // Display ASCII Art
        wattron(mainWindow, COLOR_PAIR(1) | A_BOLD);
        // Print "Rebirth" part
        for (int i = 0; i < rebirth_lines; ++i) {
            mvwprintw(mainWindow, art_start_y + i,
                      std::max(1, (width - (int)strlen(ascii_rebirth[i])) / 2), "%s",
                      ascii_rebirth[i]);
        }
        // Print subtitle part
        for (int i = 0; i < subtitle_lines; ++i) {
            mvwprintw(mainWindow, art_start_y + rebirth_lines + i,
                      std::max(1, (width - (int)strlen(ascii_subtitle[i])) / 2), "%s",
                      ascii_subtitle[i]);
        }
        wattroff(mainWindow, COLOR_PAIR(1) | A_BOLD);
        // menu_start_y = art_start_y + art_total_lines + 2;  // Position menu below art
        menu_start_y = height / 2 - 1;  // Position menu in the middle
    } else {
        // Terminal too small, display simple title
        const char* title = "Rebirth: Me Delivering Keeta in Doomsday";
        int titleLen = strlen(title);
        wattron(mainWindow, COLOR_PAIR(1) | A_BOLD);
        mvwprintw(mainWindow, 3, std::max(1, (width - titleLen) / 2), "%s", title);
        wattroff(mainWindow, COLOR_PAIR(1) | A_BOLD);
        menu_start_y = height / 2 - 1;  // Default position if no art
    }

    // Menu Items and Instructions
    // Adjust positions based on whether ASCII art was shown

    int menuX = std::max(1, width * 3 / 10);
    int descX = std::max(menuX + 15, width * 6 / 10);

    int current_menu_y = menu_start_y;
    for (int i = 0; i < menuItems.size(); ++i) {
        if (i == menuHighlight) {
            wattron(mainWindow, A_REVERSE | COLOR_PAIR(1));
        } else {
            wattroff(mainWindow, A_REVERSE | COLOR_PAIR(1));
        }
        // Add a small prefix for visual selection indication
        // "I like this, this is cute."
        std::string prefix = (i == menuHighlight) ? ">> " : "   ";
        mvwprintw(mainWindow, current_menu_y + i, menuX, "%s%s", prefix.c_str(),
                  menuItems[i].c_str());
    }
    wattroff(mainWindow, A_REVERSE | COLOR_PAIR(1));  // Turn off highlight after loop

    // Instructions - Use calculated menu_start_y
    int rightY = menu_start_y;  // Align instructions with menu items vertically
    wattron(mainWindow, COLOR_PAIR(2));
    mvwprintw(mainWindow, rightY++, descX, "-------------------------");
    mvwprintw(mainWindow, rightY++, descX, "Welcome, Player!");
    mvwprintw(mainWindow, rightY++, descX, " ");
    mvwprintw(mainWindow, rightY++, descX, "Navigate: UP/DOWN Arrows");
    mvwprintw(mainWindow, rightY++, descX, "Select:   ENTER");
    mvwprintw(mainWindow, rightY++, descX, " ");
    mvwprintw(mainWindow, rightY++, descX, "Select 'New Game' to begin");
    mvwprintw(mainWindow, rightY++, descX, "your perilous journey.");
    mvwprintw(mainWindow, rightY++, descX, "Select 'Load Game' to continue your");
    mvwprintw(mainWindow, rightY++, descX, "previous progress.");
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
        std::string prefix = (i == difficultyHighlight) ? ">> " : "   ";  // Add prefix
        if (i == difficultyHighlight) {
            wattron(mainWindow, A_REVERSE | COLOR_PAIR(1));  // Use same highlight style
        } else {
            wattroff(mainWindow, A_REVERSE | COLOR_PAIR(1));  // Ensure no highlight otherwise
        }
        mvwprintw(mainWindow, startY + i, menuX, "%s%s", prefix.c_str(),
                  difficultyItems[i].c_str());
    }
    wattroff(mainWindow, A_REVERSE | COLOR_PAIR(1));  // Turn off highlight after loop

    // Diff Details
    int rightY = startY - 2;
    rightY = std::max(5, rightY);

    wattron(mainWindow, COLOR_PAIR(2));
    mvwprintw(mainWindow, rightY++, descX, "-------------------------");

    std::string map_size, packages, obstacles, desc;
    switch (difficultyHighlight) {
        // EZ
        case 0:
            map_size = "15x15";
            packages = "3";
            obstacles = "4";
            desc = "A gentler start.";
            break;
        // Medium
        case 1:
            map_size = "20x20";
            packages = "4";
            obstacles = "5";
            desc = "A standard challenge.";
            break;
        // Hard
        case 2:
            map_size = "25x25";
            packages = "5";
            obstacles = "6";
            desc = "For the seasoned courier.";
            break;
    }

    mvwprintw(mainWindow, rightY++, descX, "Difficulty: %s",
              difficultyItems[difficultyHighlight].c_str());
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
    int instrX = std::max(1, (width - (int)strlen(instructions)) / 2);  // Center these texts
    int instrY = height - 3;
    if (instrY <= rightY) {
        instrY = rightY + 1;
    }
    // Off screen test
    instrY = std::min(height - 2, instrY);

    mvwprintw(mainWindow, instrY, instrX, "%s", instructions);

    wrefresh(mainWindow);
}

void Game::displayContent(const std::string& text) {
    werase(mainWindow);
    box(mainWindow, 0, 0);
    mvwprintw(mainWindow, height / 2, (width - text.length()) / 2, "%s", text.c_str());
    wgetch(mainWindow);  // wgetch refreshes window by default
}

void Game::display_size_warning() {
    int term_h, term_w;
    getmaxyx(stdscr, term_h, term_w);
    erase();  // Clear stdscr
    mvprintw(term_h / 2 - 1, (term_w - 35) / 2, "Terminal too small. Please resize.");
    mvprintw(term_h / 2, (term_w - 42) / 2, "Requires at least %d height and %d width.", MIN_HEIGHT,
             MIN_WIDTH);
    refresh();  // Refresh stdscr to show the warning
}

bool Game::checkSize() {
    int newHeight, newWidth;
    getmaxyx(stdscr, newHeight, newWidth);

    if (newHeight < MIN_HEIGHT || newWidth < MIN_WIDTH) {
        return false;  // Size is too small = return false
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
        mvwin(mainWindow, 0, 0);  // Ensure window is at top-left after resize

        // Re-draw necessary elements after resize
        // Mark the window and its background for complete redraw
        touchwin(stdscr);
        touchwin(mainWindow);
        wnoutrefresh(stdscr);
        werase(mainWindow);
        box(mainWindow, 0, 0);  // Redraw box
        wnoutrefresh(mainWindow);
        doupdate();
    }
    return true;  // Size is okay = return true
}

void Game::waitForResize() {
    display_size_warning();  // Show warning on stdscr

    nodelay(stdscr, TRUE);  // Set non-blocking mode for stdscr

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

void Game::newGame(const int difficultyHighlight, bool isNewGame) {
    // TODO: Initialize game state based on difficulty (map size, packages, etc.)
    // TODO: Enter the actual game loop here (or elsewhere, I might be reconstructing it soon)
    Gameplay gameplay(difficultyHighlight, current_state, isNewGame);
    gameplay.run();
}

void Game::displayStats() {
    displayContent("Displaying Stats... (Not implemented)");
    // TODO: Implement stats display logic
    // For now, just show a message and wait for input to return
}

// Main Game Loop

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

    while (current_state != GameState::EXITING) {  // Loop until exit state
        // Size Check
        if (!checkSize()) {
            waitForResize();
            // After resize, checkSize() will have redrawn the window if necessary.
            // Continue ensures the correct state's display function is called immediately.
            continue;
        }

        // Special handling for LOAD_GAME
        if (current_state == GameState::LOAD_GAME) {
            isNewGame = false;
            newGame(difficultyHighlight, isNewGame);
            
            clear();
            refresh();
            werase(mainWindow);
            box(mainWindow, 0, 0);
            displayMenu(); // Force redraw menu
            wrefresh(mainWindow);
            
            continue;  // Skip to next iteration with updated state
        }

        // Display based on State
        switch (current_state) {
            case GameState::MAIN_MENU:
                displayMenu();
                break;
            case GameState::DIFFICULTY_SELECT:
                displayDifficultyMenu();
                break;
            case GameState::LOAD_GAME:
                isNewGame = false;
                newGame(difficultyHighlight, isNewGame);
                break;
            default:
                // Shouldn't happen but just in case
                displayContent("Error: Unknown game state!");
                current_state = GameState::MAIN_MENU;  // Recover to main menu
                break;
        }

        // Input Handling - Only process input if not already handled by the display function (like
        choice = wgetch(mainWindow);  // Get input

        // State-Specific Input Processing
        switch (current_state) {
            case GameState::MAIN_MENU:
                handleMainMenuInput(choice);
                break;
            case GameState::DIFFICULTY_SELECT:
                handleDifficultyInput(choice);
                break;
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
            if (menuHighlight == 0) {  // "Start" selected
                current_state = GameState::DIFFICULTY_SELECT;
                difficultyHighlight = 0;      // Reset difficulty selection highlight
            } else if (menuHighlight == 1) {  // "Load Game" selected
                // Check if save file exists first
                std::ifstream saveFile("savegame.txt");
                if (saveFile.good()) {
                    // File exists, try to read difficulty
                    int savedDifficulty;
                    if (saveFile >> savedDifficulty) {
                        difficultyHighlight = savedDifficulty;  // Set correct difficulty
                        isNewGame = false;  // Mark as loading a game
                        saveFile.close();
                        current_state = GameState::LOAD_GAME;
                    } else {
                        saveFile.close();
                        displayContent("Save file is corrupted!");
                    }
                } else {
                    displayContent("No saved game found!");
                }
                } else if (menuHighlight == 2) {  // "Exit" selected
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
            difficultyHighlight =
                (difficultyHighlight - 1 + difficultyItems.size()) % difficultyItems.size();
            break;
        case KEY_DOWN:
            difficultyHighlight = (difficultyHighlight + 1) % difficultyItems.size();
            break;
        case '\n':
        case KEY_ENTER:
            // Pass the selected difficulty index (0=Easy, 1=Medium, 2=Hard)
            isNewGame = true; // Reset game
            newGame(difficultyHighlight, isNewGame);
            break;
        case 27:                                   // ESC
        case KEY_BACKSPACE:                        // Often mapped similarly or preferred by users
            current_state = GameState::MAIN_MENU;  // Go back to main menu
            menuHighlight = 0;
            break;
        // KEY_RESIZE is handled globally in run()
        default:
            break;
    }
}
