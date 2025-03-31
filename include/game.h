#pragma once

#include <ncurses.h>
#include <string>
#include <vector>

class Game {
private:
    WINDOW* mainWindow;
    int height;
    int width;
    int menuHighlight;
    std::vector<std::string> menuItems;
    const int MIN_HEIGHT = 20;
    const int MIN_WIDTH = 80;

public:
    // Constructor initializes ncurses and create main window
    Game() : menuHighlight(0), menuItems{"New Game", "Load", "Exit"} {
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
    ~Game() {
        delwin(mainWindow);
        endwin();
    }

    void displayMenu() {
        werase(mainWindow);
        box(mainWindow, 0, 0);

        for (size_t i = 0; i < menuItems.size(); ++i) {
            if (i == menuHighlight) {
                wattron(mainWindow, A_REVERSE);
            }
            mvwprintw(mainWindow, height / 2 - menuItems.size() / 2 + i, width / 2 - menuItems[i].length() / 2, menuItems[i].c_str());
            wattroff(mainWindow, A_REVERSE);
        }

        wrefresh(mainWindow);
    }

    void displayContent(const std::string& text) {
        werase(mainWindow);
        box(mainWindow, 0, 0);
        mvwprintw(mainWindow, height / 2, width / 2 - text.length() / 2, text.c_str());
        wgetch(mainWindow);
    }

    void display_size_warning() {
        werase(mainWindow);

        attron(A_BOLD);
        box(mainWindow, 0, 0);
        std::string warning = "WARNING: Terminal size too small!";
        mvwprintw(mainWindow, height / 2, width / 2 - warning.length() / 2, warning.c_str());
        attroff(A_BOLD);
        
        wrefresh(mainWindow);
    }
    
    bool checkSize() {
        int newHeight, newWidth;
        getmaxyx(stdscr, newHeight, newWidth);
        height = newHeight;
        width = newWidth;
        
        return (newHeight >= MIN_HEIGHT && newWidth >= MIN_WIDTH);
    }
    
    void waitForResize() {
        while (!checkSize()) {
            display_size_warning();
        }
    }

    void newGame() {
        displayContent("Starting a new game. Press any key to return to menu.");
        // Gameplay logic all handled by startGame class
    }

    void load() {
        displayContent("Loading game. Press any key to return to menu.");
        // Load game logic handled by load class
    }

    // Everything happens in here
    void run() {
        int choice;

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
};