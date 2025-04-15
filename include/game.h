#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <vector>
#include <string>

// Define the GameState enum before the class uses it
enum class GameState {
    MAIN_MENU,
    DIFFICULTY_SELECT,
    STATS,
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

    // Display Functions
    void displayMenu();
    void displayDifficultyMenu();
    void displayContent(const std::string& text);
    void display_size_warning(int &newHeight, int &newWidth);
    void displayStats(); // Changed load() to displayStats()
    
    // Game Logic Functions
    void newGame(int difficulty);
    
    // Input Handlers
    void handleMainMenuInput(int choice);
    void handleDifficultyInput(int choice);
    
    // Utility Functions
    void ask_for_resize();

};

#endif