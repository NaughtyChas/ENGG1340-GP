#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <ncurses.h>
#include <string>
#include "game.h"

class Gameplay {
public:
    Gameplay(const int &difficultyHighlight, GameState &current_state);
    ~Gameplay();
    void run();

private:
    // Member Variables
    GameState &current_state; // Initialize currentState by reference to enable direct modification
    int difficultyHighlight;
    std::string diff_str;
    int map_size;
    int num_obs;
    int num_pkg;
    int height, width;
    int roundNumber;

    // Windows
    WINDOW *mapWin;
    WINDOW *statsWin;
    WINDOW *timeWin;
    WINDOW *legendWin;

    // Private Methods
    void resizeWindows();
    void displayMap();
    void displayStats();
    void displayTime();
    void displayLegend();
    // Add methods for handling input, updating game state...
    // TBD
};

#endif