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
    Game();

    // Destructor deletes game instance when exits main
    ~Game();

    void displayMenu();
    void displayContent(const std::string& text);
    void display_size_warning();
    void displayStats(); // Changed load() to displayStats()

    // Window Resizing Functions
    void displayInitialResizePrompt();

    // Game Logic Functions
    void newGame(int difficulty);

    // Input Handlers
    void handleMainMenuInput(int choice);
    void handleDifficultyInput(int choice);

    // Utility Functions
    bool checkSize();
    void waitForResize();
    void newGame();
    void load();
    void run();
};