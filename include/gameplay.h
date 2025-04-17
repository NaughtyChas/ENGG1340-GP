#ifndef GAMEPLAY_H
#define GAMEPLAY_H

class Gameplay {
public:
    Gameplay(const int &difficultyHighlight, GameState &current_state);
    ~Gameplay();

private:
    WINDOW* mapWin;
    WINDOW* statsWin;
    WINDOW* timeWin;
    WINDOW* legendWin;

    GameState &current_state; // Initialize currentState by reference to enable direct modification
    int difficultyHighlight;

    int height, width;
    std::string diff_str;
    int map_size, num_obs, num_pkg;

    // Display functions
    void resizeWindows();
    void displayMap();
    void displayStats();
    void displayTime();
    void displayLegend();

public:
    void run();
};






#endif