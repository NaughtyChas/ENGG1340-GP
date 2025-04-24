#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <ncurses.h>
#include <string>
#include <vector>
#include <chrono>
#include "game.h"

class Gameplay {
public:
    Gameplay(const int &difficultyHighlight, GameState &current_state);
    ~Gameplay();
    void run();
    // Not yet implemented
    void addHistoryMessage(const std::string& message);

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
    std::vector<std::string> historyMessages; // To store messages
    std::chrono::steady_clock::time_point startTime;

    // Package Tracking
    std::vector<bool> hasPackage; // Tracks if player is holding package i
    int currentPackageIndex;      // Index (0 to num_pkg-1) of selected package, -1 if none
    int packagesDelivered;

    // Gameplay Map & Player
    std::vector<std::string> mapGrid;
    int playerY, playerX;
    int exitY, exitX;

    // Windows
    WINDOW *mapWin;
    WINDOW *statsWin;
    WINDOW *timeWin;
    WINDOW *legendWin;
    WINDOW *staminaWin;
    WINDOW *historyWin;
    WINDOW *packageWin;

    // Private Methods
    void initializeMap();
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
    void handleInput(int ch);
};

#endif