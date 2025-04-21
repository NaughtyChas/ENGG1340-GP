#ifndef PLAYER_H
#define PLAYER_H

#include <ncurses.h>
#include <string>
#include <vector>

class Gameplay;

class Player {
private:
    Gameplay* gameplay;
    int x;
    int y;
    char symbol;

public:
    Player(Gameplay* gameplay);
    ~Player();

    void draw();
    // Movement handling
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();

    // Package handling
    void PickupPackage();
    void dropPackage();
};

#endif