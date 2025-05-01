#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <vector>
#include <string>

// Define the GameState enum before the class uses it
enum class GameState {
    MAIN_MENU,
    DIFFICULTY_SELECT,
    LOAD_GAME,
    IN_GAME,
    EXITING
};

class Game {
public:
    Game();
    ~Game();
    void run();

private:
    WINDOW *mainWindow;
    int height, width;
    int menuHighlight;
    std::vector<std::string> menuItems;

    // Added Members
    GameState current_state; // Current state of the game
    int difficultyHighlight; // Currently selected difficulty option index
    std::vector<std::string> difficultyItems; // Difficulty level names
    bool isNewGame; // For gamesaving

    // Display Functions
    void displayMenu();
    void displayDifficultyMenu();
    void displayContent(const std::string& text);
    void display_size_warning();
    void displayStats(); // Changed load() to displayStats()

    // Window Resizing Functions
    void displayInitialResizePrompt();

    // Game Logic Functions
    void newGame(int difficulty, bool isNewGame);

    // Input Handlers
    void handleMainMenuInput(int choice);
    void handleDifficultyInput(int choice);

    // Utility Functions
    bool checkSize();
    void waitForResize();

};

#endif