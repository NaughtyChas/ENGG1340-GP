#include "../include/player.h"

#include <ncurses.h>

#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "../include/gameplay.h"

Player::Player(Gameplay* gameplay)
    : gameplay(gameplay),
      x(gameplay->getMapSize()),  // Or different spawn location everytime
      y(gameplay->getMapSize() / 2),
      symbol('@') {
}

Player::~Player() {
}

void Player::draw() {
    mvwaddch(gameplay->getMapWindow(), y, x, symbol);
}

void Player::moveUp() {
    if (gameplay->getStaminaEmpty()) {
        gameplay->addHistoryMessage("Stamina already empty!");
        return;
    }

    if (y > 1) {
        y--;
        gameplay->addHistoryMessage("Moved up");
        gameplay->decreaseStamina(10);
        return;
    }

    gameplay->addHistoryMessage("Move out of bounds");
}

void Player::moveDown() {
    if (gameplay->getStaminaEmpty()) {
        gameplay->addHistoryMessage("Stamina already empty!");
        return;
    }

    if (y < gameplay->getMapSize() - 2) {
        y++;
        gameplay->addHistoryMessage("Moved down");
        gameplay->decreaseStamina(10);
        return;
    }

    gameplay->addHistoryMessage("Move out of bounds");
}

void Player::moveLeft() {
    if (gameplay->getStaminaEmpty()) {
        gameplay->addHistoryMessage("Stamina already empty!");
        return;
    }

    if (x > 1) {
        x--;
        gameplay->addHistoryMessage("Moved left");
        gameplay->decreaseStamina(10);
        return;
    }

    gameplay->addHistoryMessage("Move out of bounds");
}

void Player::moveRight() {
    if (gameplay->getStaminaEmpty()) {
        gameplay->addHistoryMessage("Stamina already empty!");
        return;
    }

    if (x < gameplay->getMapSize() * 2 - 2) {
        x++;
        gameplay->addHistoryMessage("Moved right");
        gameplay->decreaseStamina(10);
        return;
    }

    gameplay->addHistoryMessage("Move out of bounds");
}

void Player::PickupPackage() {
}

void Player::dropPackage() {
}
