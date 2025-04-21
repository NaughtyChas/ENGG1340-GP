#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <ncurses.h>
#include <string>
#include <vector>
#include <chrono>

#include "game.h"
#include "player.h"

class Gameplay {
public:
    Gameplay(const int &difficultyHighlight, GameState &current_state);
    ~Gameplay();
    void run();
    // Not yet implemented
    void addHistoryMessage(const std::string& message);
    void decreaseStamina(const int amount);
    void increaseStamina(const int amount);

    // Getter methods
    WINDOW* getMapWindow() const { return mapWin; }
    int getMapSize() const { return map_size; }
    bool getStaminaEmpty() const { return staminaEmpty; }
    bool getStaminaFull() const { return staminaFull; }

private:
    // Member Variables
    GameState &current_state; // Initialize current_State by reference to enable direct modification
    int difficultyHighlight;
    std::string diff_str;
    int map_size;
    int num_obs;
    int num_pkg;
    int height, width;
    int roundNumber;
    int currentStamina;
    int maxStamina;
    bool staminaChanged; // Not sure if really need this
    bool staminaEmpty;
    bool staminaFull;
    std::vector<std::string> historyMessages; // To store messages
    std::chrono::steady_clock::time_point startTime;
    Player* player; // Create player 

    // Package Tracking
    std::vector<bool> hasPackage;
    int currentPackageIndex;

    // Windows
    WINDOW *mapWin;
    WINDOW *statsWin;
    WINDOW *timeWin;
    WINDOW *legendWin;
    WINDOW *staminaWin;
    WINDOW *historyWin;
    WINDOW *packageWin;

    // Private Methods
    void resizeWindows();
    void displayMap();
    void displayStats();
    void displayTime();
    void displayLegend();
    void displayStaminaBar();
    // Add methods for handling input, updating game state...
    // TBD
    void displayHistory();
    void displayPackages();
};

#endif