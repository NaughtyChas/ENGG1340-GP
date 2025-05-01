#include "../include/gameplay.h"

#include <ncurses.h>

#include <algorithm>
#include <chrono>  // For timing
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../include/game.h"

// Map initialization helper functions
void Gameplay::initializeMap() {
    mapGrid.assign(map_size, std::string(map_size, '.'));

    // Top and Bottom borders
    for (int x = 0; x < map_size; ++x) {
        mapGrid[0][x] = '-';
        mapGrid[map_size - 1][x] = '-';
    }
    // Left and Right borders
    for (int y = 1; y < map_size - 1; ++y) {
        mapGrid[y][0] = '|';
        mapGrid[y][map_size - 1] = '|';
    }
    // Corners
    mapGrid[0][0] = '+';
    mapGrid[0][map_size - 1] = '+';
    mapGrid[map_size - 1][0] = '+';
    mapGrid[map_size - 1][map_size - 1] = '+';

    // Seed
    srand(time(0));

    // Create a random number engine for std::shuffle
    std::random_device rd;
    std::mt19937 g(rd());

    // Reset Delivered Count for New Round
    packagesDelivered = 0;
    packagePickUpLocs.clear();
    packageDestLocs.clear();
    packagePickUpLocs.resize(num_pkg);
    packageDestLocs.resize(num_pkg);
    supplyStationLocations.clear();
    speedBumpLocations.clear();

    // Define Player Start and Exit Locations
    playerY = map_size / 2;
    playerX = 1;
    exitY = map_size / 2;
    exitX = map_size - 2;
    mapGrid[exitY][exitX] = 'Q';  // Place exit marker

    // Helper Lambdas
    auto isValidInner = [&](int r, int c) {
        return r > 0 && r < map_size - 1 && c > 0 && c < map_size - 1;
    };
    auto isOccupiedOrProtected = [&](int r, int c) {
        if (!isValidInner(r, c))
            return true;
        if (mapGrid[r][c] != '.')
            return true;
        if (r == playerY && c == playerX)
            return true;
        if (r == exitY && c == exitX)
            return true;
        for (const auto& loc : packagePickUpLocs)
            if (r == loc.first && c == loc.second)
                return true;
        for (const auto& loc : packageDestLocs)
            if (r == loc.first && c == loc.second)
                return true;
        return false;
    };
    auto isValidObstacle = [&](int r, int c) {
        if (!(r > 1 && r < map_size - 2 && c > 1 && c < map_size - 2))
            return false;
        for (int dr = -1; dr < 2; dr++) {
            for (int dc = -1; dc < 2; dc++) {
                int nr = r + dr;
                int nc = c + dc;
                if (nr >= 0 && nr < map_size && nc >= 0 && nc < map_size) {
                    if (mapGrid[nr][nc] == '#')
                        return false;
                }
            }
        }
        return true;
    };

    // Generate Package Pickup Locations
    int packagesPlaced = 0;
    while (packagesPlaced < num_pkg) {
        int y = (rand() % (map_size - 2)) + 1;
        int x = (rand() % (map_size - 2)) + 1;
        if (mapGrid[y][x] == '.' && !(y == playerY && x == playerX)) {
            bool validPackageLocation = true;
            for (int i = 0; i < packagesPlaced; ++i) {
                if ((packagePickUpLocs[i].first == y && packagePickUpLocs[i].second == x) ||
                    invalidPackageDistance(y, x, i)) {
                    validPackageLocation = false;
                    break;
                }
            }
            if (validPackageLocation) {
                packagePickUpLocs[packagesPlaced] = {y, x};
                mapGrid[y][x] = 'O';
                packagesPlaced++;
            }
        }
    }

    // Generate Corresponding Destination Locations
    int destinationsPlaced = 0;
    while (destinationsPlaced < num_pkg) {
        int y = (rand() % (map_size - 2)) + 1;
        int x = (rand() % (map_size - 2)) + 1;
        if (mapGrid[y][x] == '.') {
            bool already_chosen = false;
            bool tooCloseToDestination = false;
            if (invalidDestinationDistance(y, x, destinationsPlaced)) {
                tooCloseToDestination = true;
            }
            for (int i = 0; i < destinationsPlaced; ++i) {
                if (packageDestLocs[i].first == y && packageDestLocs[i].second == x) {
                    already_chosen = true;
                    break;
                }
            }
            if (!(already_chosen || tooCloseToDestination)) {
                packageDestLocs[destinationsPlaced] = {y, x};
                mapGrid[y][x] = 'X';
                destinationsPlaced++;
            }
        }
    }

    // Obstacle Generation
    int obstaclePlaced = 0;

    int numObstaclesToPlace = 4;  // Default Easy
    int numClusters = 3;
    int clusterSize = 2;      // Default Easy
    int maxBlocksPerRow = 2;  // Default Easy
    int maxObstacleLength = 5;
    int minObstacleLength = 3;

    if (difficultyHighlight == 1) {  // Medium
        numObstaclesToPlace = 5;
        clusterSize = 3;
        maxBlocksPerRow = 3;
        minObstacleLength = 7;
        maxObstacleLength = 10;
    } else if (difficultyHighlight == 2) {  // Hard
        numObstaclesToPlace = 5;
        clusterSize = 4;
        maxBlocksPerRow = 4;
        minObstacleLength = 8;
        maxObstacleLength = 12;
    }

    // Stripes placement
    int maxPlacementAttempts = map_size * map_size * 2;  // Limit attempts
    int placementAttempts = 0;

    while (obstaclePlaced < numObstaclesToPlace && placementAttempts < maxPlacementAttempts) {
        placementAttempts++;
        bool horizontal = (rand() % 2 == 0);  // Random orientation
        int len = minObstacleLength +
                  (rand() % (maxObstacleLength - minObstacleLength + 1));  // Random length

        int startY = (rand() % (map_size - len - 2)) + 1;
        int startX = (rand() % (map_size - len - 2)) + 1;

        bool canPlace = true;
        std::vector<std::pair<int, int>> currentObstacleCoords;

        for (int i = 0; i < len; ++i) {
            int currentY = startY + (horizontal ? 0 : i);
            int currentX = startX + (horizontal ? i : 0);

            if (!isValidObstacle(currentY, currentX) || mapGrid[currentY][currentX] != '.') {
                canPlace = false;
                break;  // Stop checking this potential obstacle
            }
            currentObstacleCoords.push_back({currentY, currentX});
        }

        if (canPlace) {
            for (const auto& coord : currentObstacleCoords) {
                mapGrid[coord.first][coord.second] = '#';
            }

            obstaclePlaced++;
        }
    }

    // Blocks placement
    int clustersPlaced = 0;
    int maxClusterAttempts = map_size * map_size;
    int clusterAttempts = 0;

    while (clustersPlaced < numClusters && clusterAttempts < maxClusterAttempts) {
        clusterAttempts++;

        // Select a random starting position for this cluster
        int startY = (rand() % (map_size - clusterSize - 2)) + 1;
        int startX = (rand() % (map_size - clusterSize - 2)) + 1;

        // Check if the entire area is valid for a cluster
        bool validClusterArea = false;
        for (int dy = 0; dy < clusterSize && !validClusterArea; dy++) {
            for (int dx = 0; dx < clusterSize && !validClusterArea; dx++) {
                int y = startY + dy;
                int x = startX + dx;
                if (isValidObstacle(y, x)) {
                    validClusterArea = true;
                    break;
                }
            }
        }

        if (!validClusterArea)
            continue;

        // Check if the area is free of other obstacles
        bool canPlaceCluster = true;
        for (int dy = 0; dy < clusterSize && canPlaceCluster; dy++) {
            for (int dx = 0; dx < clusterSize && canPlaceCluster; dx++) {
                int y = startY + dy;
                int x = startX + dx;
                if (isOccupiedOrProtected(y, x)) {
                    canPlaceCluster = false;
                }
            }
        }

        if (canPlaceCluster) {
            // Skip individual isValidObstacle checks
            bool placedAnyBlocks = false;

            // Generate pattern
            for (int dy = 0; dy < clusterSize; dy++) {
                int blocksInRow = 1 + (rand() % maxBlocksPerRow);
                blocksInRow = std::min(blocksInRow, clusterSize);

                std::vector<int> positions;
                for (int i = 0; i < clusterSize; i++) {
                    positions.push_back(i);
                }
                // Shuffle to randomize position selection
                std::shuffle(positions.begin(), positions.end(), g);

                // Place blocks directly without checking isValidObstacle again
                for (int b = 0; b < blocksInRow; b++) {
                    int y = startY + dy;
                    int x = startX + positions[b];

                    if (!isOccupiedOrProtected(y, x)) {
                        mapGrid[y][x] = '#';
                        placedAnyBlocks = true;
                    }
                }
            }

            if (placedAnyBlocks) {
                clustersPlaced++;
            }
        }
    }

    // --- Place Supply Station [$] ---
    int numStationsToPlace = 1;      // Default Easy
    if (difficultyHighlight == 1) {  // Medium
        numStationsToPlace = 2;
    } else if (difficultyHighlight == 2) {  // Hard
        numStationsToPlace = 3;
    }

    int stationsPlaced = 0;
    int supplyAttempts = 0;
    int maxSupplyAttempts = map_size * map_size * 2;

    while (stationsPlaced < numStationsToPlace && supplyAttempts < maxSupplyAttempts) {
        supplyAttempts++;
        int y = (rand() % (map_size - 2)) + 1;
        int x = (rand() % (map_size - 4)) + 1;

        if (mapGrid[y][x] == '.' && !isOccupiedOrProtected(y, x) && mapGrid[y][x + 1] == '.' &&
            !isOccupiedOrProtected(y, x + 1) && mapGrid[y][x + 2] == '.' &&
            !isOccupiedOrProtected(y, x + 2)) {
            mapGrid[y][x] = '[';
            mapGrid[y][x + 1] = '$';
            mapGrid[y][x + 2] = ']';

            supplyStationLocations.push_back({y, x});
            stationsPlaced++;
        }
    }

    // --- Place Speed Bumps [~] ---
    int numPatches;
    int minY, maxY;
    int minX, maxX;

    switch (difficultyHighlight) {
        case 0:                             // Easy
            numPatches = 2 + (rand() % 2);  // 2-3 patches
            minY = 2;
            maxY = 4;
            minX = 2;
            maxX = 4;
            break;

        case 1:                             // Medium
            numPatches = 3 + (rand() % 2);  // 3-4 patches
            minY = 3;
            maxY = 5;
            minX = 3;
            maxX = 5;
            break;

        case 2:                             // Hard
            numPatches = 4 + (rand() % 2);  // 4-5 patches
            minY = 4;
            maxY = 6;
            minX = 4;
            maxX = 6;
            break;

        default:
            numPatches = 2 + (rand() % 2);
            minY = 1;
            maxY = 4;
            minX = 2;
            maxX = 4;
            break;
    }

    int patchesPlaced = 0;
    int maxAttempts = map_size * map_size * 2;
    int attempts = 0;

    while (patchesPlaced < numPatches && attempts < maxAttempts) {
        attempts++;

        int patchRows = minY + (rand() % (maxY - minY + 1));

        int patchStartY = (rand() % (map_size - patchRows - 2)) + 1;
        int patchStartX = (rand() % (map_size - maxX - 2)) + 1;

        bool placedPatch = false;

        for (int row = 0; row < patchRows; row++) {
            // Randomize starting x with slight offset
            int rowStartX = patchStartX + rand() % 3;  // 0, 1 or 2 offset

            int rowLength = minX + (rand() % (maxX - minX + 1));

            for (int col = 0; col < rowLength; col++) {
                int y = patchStartY + row;
                int x = rowStartX + col;

                if (!isOccupiedOrProtected(y, x)) {
                    mapGrid[y][x] = '~';
                    speedBumpLocations.push_back({y, x});
                    placedPatch = true;
                }
            }
        }

        if (placedPatch)
            patchesPlaced++;
    }

    stepsTakenThisRound = 0;
    startTime = std::chrono::steady_clock::now();
}

// Constructor initializes windows based on difficulty
Gameplay::Gameplay(const int& difficultyHighlight, GameState& current_state, bool isNewGame)
    : difficultyHighlight(difficultyHighlight),
      current_state(current_state),
      map_size(0),
      num_obs(0),
      num_pkg(0),
      roundNumber(1),
      currentStamina(200),
      maxStamina(200),
      staminaAtRoundStart(200),
      stepsTakenThisRound(0),
      totalScore(0),
      lastRoundStepScore(0),
      lastRoundTimeScore(0),
      currentPackageIndex(-1),
      packagesDelivered(0),
      playerY(0),
      playerX(0),
      exitY(0),
      exitX(0),
      doubleStaminaCostNextMove(false) {
    switch (difficultyHighlight) {
        case 0:
            diff_str = "Easy";
            map_size = 15;  // Adjust in future
            num_obs = 5;
            num_pkg = 3;
            break;
        case 1:  // Medium
            diff_str = "Medium";
            map_size = 20;  // Same above
            num_obs = 6;
            num_pkg = 4;
            break;
        case 2:  // Hard
            diff_str = "Hard";
            map_size = 25;
            num_obs = 7;
            num_pkg = 5;
            break;
        default:
            diff_str = "Unknown";
            map_size = 15;
            num_obs = 5;
            num_pkg = 3;
            break;
    }

    if (isNewGame) {
        // Initialize as a new game
        roundNumber = 1;
        currentStamina = 200;
        maxStamina = 200;
        staminaAtRoundStart = 200;
        totalScore = 0;
        lastRoundStepScore = 0;
        lastRoundTimeScore = 0;
    } else {
        // Load from save file
        loadGameState();
    }

    hasPackage.resize(num_pkg, false);

    // Initialize the map grid
    initializeMap();

    nodelay(stdscr, TRUE);
    getmaxyx(stdscr, height, width);

    // Initialize windows
    mapWin = newwin(1, 1, 0, 0);  // Dummy sizes
    statsWin = newwin(1, 1, 0, 0);
    timeWin = newwin(1, 1, 0, 0);
    legendWin = newwin(1, 1, 0, 0);
    staminaWin = newwin(3, 1, 0, 0);
    historyWin = newwin(1, 1, 0, 0);
    packageWin = newwin(3, 1, 0, 0);

    keypad(stdscr, TRUE);
}

Gameplay::~Gameplay() {
    // Restore blocking input
    nodelay(stdscr, FALSE);

    // Delete all windows
    delwin(mapWin);
    delwin(statsWin);
    delwin(timeWin);
    delwin(legendWin);
    delwin(staminaWin);
    delwin(historyWin);
    delwin(packageWin);

    clear();
    refresh();
}

void Gameplay::resizeWindows() {
    getmaxyx(stdscr, height, width);

    // Recalculate window positions and sizes
    // --- Map Window ---
    int mapHeight = map_size;
    int mapWidth = map_size * 2;
    int mapY = std::max(0, (height - mapHeight) / 2);
    int mapX = std::max(0, (width - mapWidth) / 2);

    // --- Side Panel Widths ---
    int sidePanelWidth = width / 4;
    int legendWidth = sidePanelWidth;
    int timeWidth = sidePanelWidth;
    int statsWidth = sidePanelWidth;
    int historyWidth = legendWidth;

    // --- Bottom Panel Heights ---
    int bottomPanelHeight = 3;  // Common height for stamina and package

    // --- Calculate Bottom Panel Widths ---
    int staminaWidth = std::max(20, width / 3);
    // Calculate desired package width
    int packageWidth = 4 + (num_pkg * 2) + 4;
    packageWidth = std::max(15, packageWidth);

    // --- Adjust widths if total exceeds screen width ---
    int totalBottomWidth = staminaWidth + packageWidth;
    if (totalBottomWidth > width) {
        double ratio = static_cast<double>(width) / totalBottomWidth;
        staminaWidth = std::max(10, static_cast<int>(staminaWidth * ratio));
        packageWidth = width - staminaWidth;  // Give remaining space to package
        packageWidth = std::max(1, packageWidth);
    }
    staminaWidth = std::max(1, staminaWidth);

    // Centered Starting X for Bottom Panel
    int bottomStartX = std::max(0, (width - (staminaWidth + packageWidth)) / 2);

    // --- Stamina Window Position  ---
    int staminaY = height - bottomPanelHeight;
    int staminaX = bottomStartX;

    // --- Package Window Position  ---
    int packageHeight = bottomPanelHeight;
    int packageY = height - bottomPanelHeight;
    int packageX = staminaX + staminaWidth;

    // --- Side Panel Heights & Positions ---
    // Left Side
    int minLegendHeight = 26;
    int desiredLegendHeight = std::min(height - bottomPanelHeight, height / 2);
    int legendHeight = std::max(minLegendHeight, desiredLegendHeight);  // Ensure minimum height
    legendHeight = std::min(height - bottomPanelHeight,
                            legendHeight);  // Don't exceed available space above bottom bar

    int legendX = 0;
    int legendY = 0;

    // --- History Window ---
    int historyY = legendHeight;
    int historyHeight = std::max(1, height - legendHeight - bottomPanelHeight);
    int historyX = 0;

    // Right Side
    int timeHeight = std::min(height - bottomPanelHeight, height / 4);
    int timeX = std::max(0, width - timeWidth);
    int timeY = 0;

    int statsX = timeX;
    int statsY = timeY + timeHeight;
    int statsHeight = std::max(1, height - timeHeight - bottomPanelHeight);

    // --- Resize and Reposition ---
    wresize(mapWin, mapHeight, mapWidth);
    mvwin(mapWin, mapY, mapX);

    wresize(legendWin, legendHeight, legendWidth);
    mvwin(legendWin, legendY, legendX);

    wresize(historyWin, historyHeight, historyWidth);
    mvwin(historyWin, historyY, historyX);

    wresize(timeWin, timeHeight, timeWidth);
    mvwin(timeWin, timeY, timeX);

    wresize(statsWin, statsHeight, statsWidth);
    mvwin(statsWin, statsY, statsX);

    // Note: With the map centered, the side panels (Legend, Time, Stats)
    // might overlap the map if the terminal width is not large enough
    // maybe will carryout a check here to ensure that the map is not overlapped
    // Reposition Stamina window using bottomStartX
    wresize(staminaWin, bottomPanelHeight, staminaWidth);
    mvwin(staminaWin, staminaY, staminaX);

    // Reposition Package window using calculated packageX
    wresize(packageWin, packageHeight, packageWidth);
    mvwin(packageWin, packageY, packageX);
}

// --- Input handling ---
void Gameplay::handleInput(int ch) {
    int nextY = playerY;
    int nextX = playerX;
    bool moved = false;

    switch (ch) {
        case 'w':     // Move Up
        case KEY_UP:  // Also handle arrow key
            nextY--;
            moved = true;
            break;
        case 's':  // Move Down
        case KEY_DOWN:
            nextY++;
            moved = true;
            break;
        case 'a':  // Move Left
        case KEY_LEFT:
            nextX--;
            moved = true;
            break;
        case 'd':  // Move Right
        case KEY_RIGHT:
            nextX++;
            moved = true;
            break;

        // --- Exit Interaction ---
        case '\n':       // Enter key
        case KEY_ENTER:  // Ncurses specific Enter key
            if (playerY == exitY && playerX == exitX) {
                // Check if all packages are delivered
                if (packagesDelivered >= num_pkg) {
                    // --- Calculate Stats & Score ---
                    int staminaReward = 50;
                    int oldStamina = currentStamina;
                    int finalStamina = std::min(maxStamina, currentStamina + staminaReward);
                    int staminaUsedThisRound = std::max(0, staminaAtRoundStart - oldStamina);

                    auto now = std::chrono::steady_clock::now();
                    auto elapsed =
                        std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
                    long long timeTaken = elapsed.count();

                    // --- Scoring ---
                    // Define base scores and penalties
                    const int BASE_STEP_SCORE = 1000;
                    const int STEP_PENALTY = 5;
                    const int BASE_TIME_SCORE = 1000;
                    const int TIME_PENALTY = 2;

                    int stepScore =
                        std::max(0, BASE_STEP_SCORE - (stepsTakenThisRound * STEP_PENALTY));
                    int timeScore =
                        std::max(0, BASE_TIME_SCORE - (static_cast<int>(timeTaken) * TIME_PENALTY));
                    int roundScore = stepScore + timeScore;

                    lastRoundStepScore = stepScore;
                    lastRoundTimeScore = timeScore;

                    totalScore += roundScore;

                    // --- Prepare Popup Message ---
                    std::vector<std::string> popupLines;
                    popupLines.push_back("Round " + std::to_string(roundNumber) + " Complete!");
                    popupLines.push_back("");
                    popupLines.push_back("Time Taken: " + std::to_string(timeTaken) +
                                         "s (Score: " + std::to_string(timeScore) + ")");
                    popupLines.push_back("Steps Taken: " + std::to_string(stepsTakenThisRound) +
                                         " (Score: " + std::to_string(stepScore) + ")");
                    popupLines.push_back("Stamina Used: " + std::to_string(staminaUsedThisRound));
                    popupLines.push_back("Stamina Bonus: +" + std::to_string(staminaReward));
                    popupLines.push_back("Round Score: " + std::to_string(roundScore));
                    popupLines.push_back("Total Score: " + std::to_string(totalScore));

                    // --- Display Popup ---
                    displayPopupMessage("Level Complete", popupLines);

                    // --- Apply Reward and Proceed ---
                    currentStamina = finalStamina;
                    staminaAtRoundStart = currentStamina;
                    addHistoryMessage("Level Complete! +" + std::to_string(staminaReward) +
                                      " stamina bonus. Round Score: " + std::to_string(roundScore));
                    roundNumber++;
                    addHistoryMessage("Proceeding to Round " + std::to_string(roundNumber) + "...");
                    initializeMap();

                    std::fill(hasPackage.begin(), hasPackage.end(), false);
                    currentPackageIndex = -1;
                    doubleStaminaCostNextMove = false;

                } else {
                    addHistoryMessage("Cannot exit yet! Deliver all packages first. (" +
                                      std::to_string(packagesDelivered) + "/" +
                                      std::to_string(num_pkg) + " delivered)");
                }
            } else {
                // Optional: Message if Enter pressed not at exit
            }
            break;

        // --- Package Selection ---
        case '1':
            if (num_pkg >= 1)
                currentPackageIndex = 0;
            addHistoryMessage("Selected package 1.");
            break;
        case '2':
            if (num_pkg >= 2)
                currentPackageIndex = 1;
            addHistoryMessage("Selected package 2.");
            break;
        case '3':
            if (num_pkg >= 3)
                currentPackageIndex = 2;
            addHistoryMessage("Selected package 3.");
            break;
        case '4':
            if (num_pkg >= 4)
                currentPackageIndex = 3;
            addHistoryMessage("Selected package 4.");
            break;
        case '5':
            if (num_pkg >= 5)
                currentPackageIndex = 4;
            addHistoryMessage("Selected package 5.");
            break;

        // --- Package Pickup ---
        case 'q': {
            bool foundPackage = false;
            for (int i = 0; i < num_pkg; ++i) {
                // Check if player is at pickup location i AND it's still on the map
                if (playerY == packagePickUpLocs[i].first &&
                    playerX == packagePickUpLocs[i].second && mapGrid[playerY][playerX] == 'O') {
                    if (!hasPackage[i]) {
                        hasPackage[i] = true;
                        mapGrid[playerY][playerX] = '.';
                        currentPackageIndex = i;
                        addHistoryMessage("Picked up package " + std::to_string(i + 1) + ".");
                        foundPackage = true;
                    } else {
                        addHistoryMessage("Already holding package " + std::to_string(i + 1) + ".");
                        foundPackage = true;
                    }
                    break;
                }
            }
            if (!foundPackage && mapGrid[playerY][playerX] == 'O') {
                addHistoryMessage("Error: Package 'O' found but no matching location data.");
            } else if (!foundPackage) {
                addHistoryMessage("No package to pick up here.");
            }
        } break;

        // --- Package Drop ---
        case 'e':
            // Check if a package is selected and held
            if (currentPackageIndex != -1 && hasPackage[currentPackageIndex]) {
                int pkgIdx = currentPackageIndex;

                // --- Prevent dropping at the exit location ---
                if (playerY == exitY && playerX == exitX) {
                    addHistoryMessage("Cannot drop packages at the exit 'Q'.");
                }
                // --- Check if the current location is empty ground '.' ---
                else if (mapGrid[playerY][playerX] == '.') {
                    // Drop the package
                    addHistoryMessage("Dropped package " + std::to_string(pkgIdx + 1) + ".");
                    hasPackage[pkgIdx] = false;
                    mapGrid[playerY][playerX] = 'O';

                    // Update the pickup location to the drop location
                    // This ensures the correct color is shown and it can be picked up again
                    packagePickUpLocs[pkgIdx] = {playerY, playerX};

                    // Find next held package or set to -1
                    currentPackageIndex = -1;
                    for (int i = 0; i < num_pkg; ++i) {
                        if (hasPackage[i]) {
                            currentPackageIndex = i;
                            break;
                        }
                    }
                }
                // --- Check if trying to drop at the correct destination 'X' ---
                else if (playerY == packageDestLocs[pkgIdx].first &&
                         playerX == packageDestLocs[pkgIdx].second &&
                         mapGrid[playerY][playerX] == 'X') {
                    // Deliver the package
                    addHistoryMessage("Delivered package " + std::to_string(pkgIdx + 1) + "!");
                    hasPackage[pkgIdx] = false;
                    packagesDelivered++;
                    mapGrid[playerY][playerX] = '.';

                    // Find next held package or set to -1
                    currentPackageIndex = -1;
                    for (int i = 0; i < num_pkg; ++i) {
                        if (hasPackage[i]) {
                            currentPackageIndex = i;
                            break;
                        }
                    }
                    // Add score in the future
                } else {
                    addHistoryMessage("Cannot drop package here. Location occupied.");
                }
            } else {
                addHistoryMessage("No package selected/held to drop.");
            }
            break;

        case 27:  // ESC
            if (displayQuitOptions()) {
                saveGameState();  // Save game data before quitting
                addHistoryMessage("Exiting to main menu...");

                clear();
                refresh();

                current_state = GameState::MAIN_MENU;

                return;
            } else {
                addHistoryMessage("Continuing game...");
            }
            break;
        case KEY_RESIZE:
            addHistoryMessage("Terminal resized.");
            clear();
            refresh();
            break;

        case ERR:
            break;

        default:
            break;
    }

    // --- Process Movement ---
    if (moved) {
        // Check Boundaries
        if (nextY > 0 && nextY < map_size - 1 && nextX > 0 && nextX < map_size - 1) {
            // Check Obstacles
            if (mapGrid[nextY][nextX] != '#') {
                // --- Calculate Stamina Cost ---
                int numHeldPackages = 0;
                for (bool held : hasPackage) {
                    if (held) {
                        numHeldPackages++;
                    }
                }
                int baseMoveCost = 1 + numHeldPackages;  // Base cost = 1 + number of packages held
                int finalMoveCost = baseMoveCost;

                if (doubleStaminaCostNextMove) {
                    finalMoveCost *= 2;
                    doubleStaminaCostNextMove =
                        false;  // Consume the flag *before* checking stamina
                }

                // Check Stamina (using calculated cost)
                if (currentStamina >= finalMoveCost) {
                    int oldStamina = currentStamina;
                    currentStamina -= finalMoveCost;
                    currentStamina =
                        std::max(0, currentStamina);  // Ensure stamina doesn't go below 0

                    // Update Player Position
                    playerY = nextY;
                    playerX = nextX;
                    stepsTakenThisRound++;

                    addHistoryMessage("Moved. Cost: " + std::to_string(finalMoveCost) +
                                      ". Stamina: " + std::to_string(oldStamina) + " -> " +
                                      std::to_string(currentStamina));

                    // --- Check for landing on Supply Station ---
                    for (int i = supplyStationLocations.size() - 1; i >= 0; --i) {
                        int stationY = supplyStationLocations[i].first;
                        int stationX = supplyStationLocations[i].second;

                        // Check if player landed on any part of this station
                        if (playerY == stationY &&
                            (playerX >= stationX && playerX <= stationX + 2)) {
                            int staminaGain = (rand() % 31) + 40;  // Ranging from 40-70
                            int oldStaminaBeforeGain = currentStamina;
                            currentStamina = std::min(maxStamina, currentStamina + staminaGain);
                            addHistoryMessage("Supply opened! +" + std::to_string(staminaGain) +
                                              " stamina. (" + std::to_string(oldStaminaBeforeGain) +
                                              "->" + std::to_string(currentStamina) + ")");

                            // Remove the supply station from the map grid
                            mapGrid[stationY][stationX] = '.';
                            mapGrid[stationY][stationX + 1] = '.';
                            mapGrid[stationY][stationX + 2] = '.';

                            // Remove the station from the active list
                            supplyStationLocations.erase(supplyStationLocations.begin() + i);

                            break;  // Player can only use one station per step
                        }
                    }

                    // --- Check for landing on Speed Bump ---
                    if (mapGrid[playerY][playerX] == '~') {
                        if (!doubleStaminaCostNextMove) {
                            addHistoryMessage("Stepped on a speed bump! Next move costs double.");
                            doubleStaminaCostNextMove = true;
                        }
                    }

                    // --- Check for Game Over (Stamina Depleted AFTER move) ---
                    if (currentStamina <= 0) {
                        // --- Prepare Popup Message ---
                        auto now = std::chrono::steady_clock::now();
                        auto elapsed =
                            std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
                        long long timeTaken = elapsed.count();

                        std::vector<std::string> popupLines;
                        popupLines.push_back("You ran out of stamina!");
                        popupLines.push_back("");
                        popupLines.push_back("Round Reached: " + std::to_string(roundNumber));
                        popupLines.push_back("Time This Round: " + std::to_string(timeTaken) + "s");
                        popupLines.push_back("Steps This Round: " +
                                             std::to_string(stepsTakenThisRound));
                        popupLines.push_back("Final Total Score: " + std::to_string(totalScore));

                        // --- Display Popup ---
                        displayPopupMessage("Game Over", popupLines);

                        // --- Set Game State ---
                        addHistoryMessage("GAME OVER! You ran out of stamina. Final Score: " +
                                          std::to_string(totalScore));
                        current_state = GameState::MAIN_MENU;
                        return;  // Exit handleInput early
                    }

                } else {  // Not enough stamina for the attempted move
                    addHistoryMessage("Cannot move! Need " + std::to_string(finalMoveCost) +
                                      " stamina, have " + std::to_string(currentStamina) + ".");
                    // Reset flag if player couldn't make the double-cost move

                    // --- Check for Softlock Game Over ---
                    // Can the player potentially resolve this by dropping a package?
                    bool canDrop = false;
                    // Check if holding a package AND on an empty '.' spot
                    if (currentPackageIndex != -1 && hasPackage[currentPackageIndex] &&
                        mapGrid[playerY][playerX] == '.') {
                        canDrop = true;
                    }

                    // If stamina is positive, but can't afford the move AND cannot drop a package,
                    // it's game over.
                    if (currentStamina > 0 && !canDrop) {
                        // --- Prepare Popup Message ---
                        auto now = std::chrono::steady_clock::now();
                        auto elapsed =
                            std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
                        long long timeTaken = elapsed.count();

                        std::vector<std::string> popupLines;
                        popupLines.push_back("You got stuck with no possible moves!");
                        popupLines.push_back("(Not enough stamina to move, cannot drop package)");
                        popupLines.push_back("");
                        popupLines.push_back("Round Reached: " + std::to_string(roundNumber));
                        popupLines.push_back("Time This Round: " + std::to_string(timeTaken) + "s");
                        popupLines.push_back("Steps This Round: " +
                                             std::to_string(stepsTakenThisRound));
                        popupLines.push_back("Final Total Score: " + std::to_string(totalScore));

                        // --- Display Popup ---
                        displayPopupMessage("Game Over", popupLines);

                        // --- Set Game State ---
                        addHistoryMessage("GAME OVER! Stuck with no possible moves. Final Score: " +
                                          std::to_string(totalScore));
                        current_state = GameState::MAIN_MENU;
                        return;
                    }
                    // --- End Softlock Check ---

                    // Reset speed bump flag if player couldn't make the double-cost move
                    if (finalMoveCost > baseMoveCost)
                        doubleStaminaCostNextMove = true;  // Put the flag back
                }
            } else {  // Hit obstacle
                addHistoryMessage("Cannot move! Blocked by obstacle.");
                // If player intended a double-cost move but hit a wall, reset the flag
                if (doubleStaminaCostNextMove) {
                    doubleStaminaCostNextMove = false;
                }
            }
        } else {
            addHistoryMessage("Cannot move! Hit the border.");
            // If player intended a double-cost move but hit border, reset the flag
            if (doubleStaminaCostNextMove) {
                doubleStaminaCostNextMove = false;
            }
        }
    }
}

void Gameplay::run() {
    // Clear remnants from mainWindow
    clear();
    refresh();

    // Initialize colors if not done elsewhere (ensure start_color() was called)
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_YELLOW, COLOR_BLACK);  // Stamina Bar
        init_pair(2, COLOR_CYAN, COLOR_BLACK);    // Player
        init_pair(3, COLOR_BLUE, COLOR_BLACK);    // Background dots '.'

        // --- Package/Destination Colors (Pairs 4-8) ---
        init_pair(4, COLOR_RED, COLOR_BLACK);      // Package 1
        init_pair(5, COLOR_GREEN, COLOR_BLACK);    // Package 2
        init_pair(6, COLOR_YELLOW, COLOR_BLACK);   // Package 3
        init_pair(7, COLOR_MAGENTA, COLOR_BLACK);  // Package 4
        init_pair(8, COLOR_CYAN, COLOR_BLACK);     // Package 5
        init_pair(9, COLOR_WHITE, COLOR_BLACK);    // Supply Station [$]
        init_pair(10, COLOR_YELLOW, COLOR_BLACK);  // Speed Bump [~]
    }

    addHistoryMessage("Game Started. Round " + std::to_string(roundNumber));
    startTime = std::chrono::steady_clock::now();

    while (current_state != GameState::MAIN_MENU) {
        getmaxyx(stdscr, height, width);

        // Stage changes done to windows
        resizeWindows();
        displayMap();
        displayStats();
        displayTime();
        displayLegend();
        displayStaminaBar();
        displayHistory();
        displayPackages();

        // Update all windows at once
        doupdate();

        int ch = getch();

        // Handle Input
        handleInput(ch);

        // Check if state changed
        if (current_state == GameState::MAIN_MENU) {
            clear();
            refresh();
            break;
        }
        napms(30);
    }
}

// Display functions
void Gameplay::displayMap() {
    werase(mapWin);

    // Get window dimensions
    int maxY = getmaxy(mapWin);
    int maxX = getmaxx(mapWin);

    // Draw Map Content
    for (int y = 0; y < map_size; ++y) {
        for (int x = 0; x < map_size; ++x) {
            int winY = y;
            int winX = x * 2;
            if (winY >= 0 && winY < maxY && winX >= 0 && winX < maxX - 1) {
                char displayChar = mapGrid[y][x];
                int colorPair = 0;

                // Determine color based on character
                if (displayChar == '.') {
                    colorPair = 3;
                } else if (displayChar == 'O') {
                    // Find which package this is
                    for (int i = 0; i < num_pkg; ++i) {
                        if (packagePickUpLocs[i].first == y && packagePickUpLocs[i].second == x) {
                            colorPair = 4 + i;  // Assign color pair
                            break;
                        }
                    }
                } else if (displayChar == 'X') {
                    // Find which destination this is
                    for (int i = 0; i < num_pkg; ++i) {
                        if (packageDestLocs[i].first == y && packageDestLocs[i].second == x) {
                            colorPair = 4 + i;  // Assign color pair
                            break;
                        }
                    }
                } else if (displayChar == '[' || displayChar == '$' || displayChar == ']') {
                    // Check if it's part of any *active* supply station in the vector
                    bool isPartOfActiveStation = false;
                    for (const auto& stationLoc : supplyStationLocations) {
                        if (y == stationLoc.first && x >= stationLoc.second &&
                            x <= stationLoc.second + 2) {
                            isPartOfActiveStation = true;
                            break;
                        }
                    }
                    if (isPartOfActiveStation) {
                        colorPair = 9;  // Apply supply station color (Pair 9)
                    }
                    // If it's '[', '$', ']' but not in the active list, it gets default color (0)
                } else if (displayChar == '~') {
                    colorPair = 10;  // Speed bump color (Pair 10)
                } else if (displayChar == 'Q') {
                    // Optional: Add color for Exit 'Q' if desired (e.g., pair 9 or a new one)
                    // colorPair = 9;
                } else if (displayChar == '#' || displayChar == '-' || displayChar == '|' ||
                           displayChar == '+') {
                    // Optional: Add color for obstacles/borders if desired
                }

                // Apply color if specified
                if (colorPair > 0) {
                    wattron(mapWin, COLOR_PAIR(colorPair));
                }

                // Use mvwaddch for single characters
                mvwaddch(mapWin, winY, winX, displayChar);

                // Turn off color
                if (colorPair > 0) {
                    wattroff(mapWin, COLOR_PAIR(colorPair));
                }
            }
        }
    }

    // Draw Player (using color pair 2)
    int playerWinY = playerY;
    int playerWinX = playerX * 2;
    if (playerWinY >= 0 && playerWinY < maxY && playerWinX >= 0 && playerWinX < maxX - 1) {
        wattron(mapWin, COLOR_PAIR(2) | A_BOLD);
        mvwaddch(mapWin, playerWinY, playerWinX, '@');
        wattroff(mapWin, COLOR_PAIR(2) | A_BOLD);
    }

    wnoutrefresh(mapWin);
}

void Gameplay::displayStats() {
    werase(statsWin);
    box(statsWin, 0, 0);
    mvwprintw(statsWin, 0, 2, " Stats ");

    int row = 1;
    int col = 2;

    // --- Display Total Score ---
    mvwprintw(statsWin, row++, col, "Total Score:");
    mvwprintw(statsWin, row++, col, " %lld", totalScore);

    // --- Add space ---
    row++;

    // --- Display Last Round Score ---
    if (roundNumber > 1) {  // Only show if at least one round is complete
        mvwprintw(statsWin, row++, col, "Last Round Score:");
        int lastRoundScore = lastRoundStepScore + lastRoundTimeScore;
        mvwprintw(statsWin, row++, col, " %d", lastRoundScore);
        // Show breakdown
        mvwprintw(statsWin, row++, col, " (Steps:%d + Time:%d)", lastRoundStepScore,
                  lastRoundTimeScore);
    } else {
        mvwprintw(statsWin, row++, col, "Last Round Score:");
        mvwprintw(statsWin, row++, col, " N/A");
        row++;  // Keep spacing consistent
    }

    // --- Add space ---
    row++;

    // --- Display Packages Delivered (Current Round) ---
    mvwprintw(statsWin, row++, col, "Packages Delivered:");
    mvwprintw(statsWin, row++, col, " %d / %d", packagesDelivered, num_pkg);

    // --- Add space ---
    row++;

    // --- Display Steps Taken (Current Round) ---
    mvwprintw(statsWin, row++, col, "Steps This Round:");
    mvwprintw(statsWin, row++, col, " %d", stepsTakenThisRound);

    wnoutrefresh(statsWin);
}

void Gameplay::displayTime() {
    werase(timeWin);
    box(timeWin, 0, 0);
    mvwprintw(timeWin, 0, 2, " Time Info ");

    // --- Calculate Elapsed Time ---
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
    long long totalSeconds = elapsed.count();
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    // --- Format Time String (MM:SS) ---
    std::ostringstream timeStream;
    timeStream << std::setw(2) << std::setfill('0') << minutes << ":" << std::setw(2)
               << std::setfill('0') << seconds;
    std::string timeStr = timeStream.str();

    // --- Display Information ---
    int row = 2;
    int col = 2;
    mvwprintw(timeWin, row++, col, "Elapsed: %s", timeStr.c_str());
    wnoutrefresh(timeWin);
}

void Gameplay::displayLegend() {
    werase(legendWin);
    box(legendWin, 0, 0);

    // Get window height to prevent writing over borders
    int winHeight = getmaxy(legendWin);

    // Display Round Number
    std::string roundText = "Round " + std::to_string(roundNumber);
    wattron(legendWin, A_BOLD);
    // Ensure title doesn't overwrite corners if window is very narrow
    int titleX = std::max(1, (getmaxx(legendWin) - static_cast<int>(roundText.length()) - 2) / 2);
    mvwprintw(legendWin, 0, titleX, " %s ", roundText.c_str());
    wattroff(legendWin, A_BOLD);

    // --- Starting row for content ---
    int row = 2;  // Start below top border and title line
    int col = 2;
    int lastAvailableRow = winHeight - 2;  // Last usable row before bottom border

    // --- Legend Content ---
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, "--- Legend ---");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, " @: Player");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, " #: Obstacle");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, " ~: Speed Bump");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, " [$]: Supply Station");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, " O: Package");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, " X: Destination");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, " Q: Exit");
    if (row <= lastAvailableRow)
        row++;  // Blank line

    // --- Movement Controls ---
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, "--- Movement ---");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, "   W: Move Up");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, "   S: Move Down");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, "   A: Move Left");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, "   D: Move Right");
    if (row <= lastAvailableRow)
        row++;  // Blank line

    // --- Package Controls ---
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, "--- Package ---");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, "   Q: Pick Up");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, "   E: Drop current");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, " 1-5: Select package");
    if (row <= lastAvailableRow)
        row++;  // Blank line

    // --- Game Controls ---
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, "---- Game ----");
    // Combine Enter description
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, " Enter: Next Level (at Q)");
    if (row <= lastAvailableRow)
        mvwprintw(legendWin, row++, col, " ESC: Exit to Menu");

    wnoutrefresh(legendWin);
}

void Gameplay::displayStaminaBar() {
    werase(staminaWin);
    box(staminaWin, 0, 0);

    // --- Title ---
    const char* title = " Stamina ";
    mvwprintw(staminaWin, 0, 2, title);

    // --- Bar Calculation ---
    int barWidth = getmaxx(staminaWin) - 4;
    int numFilled = 0;
    if (maxStamina > 0) {
        if (barWidth > 0) {
            numFilled = static_cast<int>(
                std::floor(static_cast<double>(currentStamina) / maxStamina * barWidth));
            numFilled = std::max(0, std::min(barWidth, numFilled));
        }
    } else {
        numFilled = 0;
    }

    // --- Draw Bar ---
    wmove(staminaWin, 1, 2);
    wattron(staminaWin, COLOR_PAIR(1));
    for (int i = 0; i < numFilled; ++i) {
        waddch(staminaWin, ACS_BLOCK);  // ▓
    }
    wattroff(staminaWin, COLOR_PAIR(1));

    // --- Numerical Display ---
    std::string staminaText = std::to_string(currentStamina) + " / " + std::to_string(maxStamina);
    int textX = getmaxx(staminaWin) - 2 - staminaText.length();
    textX = std::max(2, textX);  // Ensure it doesn't overwrite left border
    mvwprintw(staminaWin, 1, textX, staminaText.c_str());

    wnoutrefresh(staminaWin);
}

void Gameplay::displayHistory() {
    werase(historyWin);
    box(historyWin, 0, 0);

    // --- Title ---
    const char* title = " Gameplay History ";
    mvwprintw(historyWin, 0, 2, title);

    // --- Display Messages ---
    int maxLines = getmaxy(historyWin) - 2;
    int startIdx = 0;
    if (historyMessages.size() > maxLines) {
        startIdx = historyMessages.size() - maxLines;
    }

    int currentLine = 1;  // Start drawing from line 1
    for (size_t i = startIdx; i < historyMessages.size(); ++i) {
        // Truncate message if too long for window width
        int maxWidth = getmaxx(historyWin) - 4;
        maxWidth = std::max(0, maxWidth);  // Ensure maxWidth is not negative
        std::string msg = historyMessages[i];
        if (msg.length() > maxWidth) {
            msg.resize(maxWidth);
        }
        mvwprintw(historyWin, currentLine++, 2, msg.c_str());
    }

    // Example placeholder if no messages yet
    if (historyMessages.empty() && maxLines > 0) {
        mvwprintw(historyWin, 1, 2, "No events yet...");
    }

    wnoutrefresh(historyWin);
}

void Gameplay::displayPackages() {
    werase(packageWin);
    box(packageWin, 0, 0);

    // --- Title ---
    const char* title = " Packages ";
    mvwprintw(packageWin, 0, 2, "%s", title);

    // --- Package Slot Display ---
    const char* emptySlot = "_";

    const char* packageChars[] = {"1", "2", "3", "4", "5"};

    int startX = 2;
    int currentX = startX;
    int yPos = 1;

    for (int i = 0; i < num_pkg; ++i) {
        const char* displayChar = emptySlot;
        int colorPair = 0;

        if (hasPackage[i]) {
            if (i < 5) {
                displayChar = packageChars[i];
            } else {
                displayChar = "?";  // Fallback if more than 5 packages somehow
            }
            colorPair = 4 + i;
        }

        // Check for highlight (selected package)
        bool isHighlighted = (i == currentPackageIndex);
        if (isHighlighted) {
            wattron(packageWin, A_REVERSE);
        }

        // Apply color if holding the package
        if (colorPair > 0) {
            wattron(packageWin, COLOR_PAIR(colorPair));
        }

        // Print the character - mvwaddstr works for both single-byte and multi-byte chars
        mvwaddstr(packageWin, yPos, currentX, displayChar);

        // Turn off color if it was applied
        if (colorPair > 0) {
            wattroff(packageWin, COLOR_PAIR(colorPair));
        }

        // Turn off highlight if it was on
        if (isHighlighted) {
            wattroff(packageWin, A_REVERSE);
        }

        // Add spacing (adjust as needed for character width)
        // Using 2 columns spacing should work reasonably for both cases
        currentX += 2;
    }

    wnoutrefresh(packageWin);
}

// Function to add a message to the history
// This function can be called from anywhere in the Gameplay class
void Gameplay::addHistoryMessage(const std::string& message) {
    historyMessages.push_back(message);
}

// Check if distance between packages generated are too close
bool Gameplay::invalidPackageDistance(const int& y, const int& x, const int& i) {
    int minDistance;
    switch (difficultyHighlight) {
        case 0:               // Easy
            minDistance = 6;  // Map will fail to generate if set too high
            break;
        case 1:  // Medium
            minDistance = 7;
            break;
        case 2:  // Hard
            minDistance = 8;
            break;
        default:
            minDistance = 6;
            break;
    }

    int y1 = y;
    int x1 = x;
    int y2 = packagePickUpLocs[i].first;
    int x2 = packagePickUpLocs[i].second;
    int currentDistance = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));

    return currentDistance < minDistance;
}

// Check if distance between individual packages and destinations is too close
bool Gameplay::invalidDestinationDistance(const int& y, const int& x,
                                          const int& destinationsPlaced) {
    int minDistance;
    switch (difficultyHighlight) {
        case 0:  // Easy
            minDistance = 8;
            break;
        case 1:  // Medium
            minDistance = 9;
            break;
        case 2:  // Hard
            minDistance = 10;
            break;
        default:
            minDistance = 8;
            break;
    }

    int y1 = y;
    int x1 = x;
    int y2 = packagePickUpLocs[destinationsPlaced].first;
    int x2 = packagePickUpLocs[destinationsPlaced].second;

    int currentDistance = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));

    return currentDistance < minDistance;
}

void Gameplay::displayPopupMessage(const std::string& title,
                                   const std::vector<std::string>& lines) {
    // --- Padding ---
    const int horizontalPadding = 3;
    const int verticalPadding = 1;

    // --- Calculate window dimensions ---
    int maxTextLength = title.length();
    for (const std::string& line : lines) {
        if (line.length() > maxTextLength) {
            maxTextLength = line.length();
        }
    }
    std::string continuePrompt = "Press any key to continue...";
    if (continuePrompt.length() > maxTextLength) {
        maxTextLength = continuePrompt.length();
    }

    int popupHeight = 1 + verticalPadding + lines.size() + verticalPadding + 1 + 1;
    int popupWidth = 1 + horizontalPadding + maxTextLength + horizontalPadding + 1;

    // Ensure dimensions are valid and don't exceed screen size
    popupHeight = std::min(popupHeight, height);
    popupWidth = std::min(popupWidth, width);
    popupHeight = std::max(5, popupHeight);
    popupWidth =
        std::max(static_cast<int>(title.length()) + (horizontalPadding * 2) + 2, popupWidth);
    popupWidth = std::max(static_cast<int>(continuePrompt.length()) + (horizontalPadding * 2) + 2,
                          popupWidth);

    // --- Position the window near the top ---
    int popupY = 1;
    int popupX = std::max(0, (width - popupWidth) / 2);

    // --- Create the window ---
    WINDOW* popupWin = newwin(popupHeight, popupWidth, popupY, popupX);
    keypad(popupWin, TRUE);
    box(popupWin, 0, 0);

    // --- Display Title (Centered within padding) ---
    int titleX =
        std::max(horizontalPadding + 1, (popupWidth - static_cast<int>(title.length())) / 2);
    wattron(popupWin, A_BOLD);
    mvwprintw(popupWin, 1 + verticalPadding, titleX, "%s", title.c_str());
    wattroff(popupWin, A_BOLD);

    // --- Display Message Lines ---
    int textStartX = 1 + horizontalPadding;
    int currentLineY = 1 + verticalPadding + 1;
    for (const std::string& line : lines) {
        if (currentLineY < popupHeight - (1 + verticalPadding + 1)) {
            std::string truncatedLine = line;
            int maxDisplayWidth = popupWidth - (horizontalPadding * 2) - 2;
            if (truncatedLine.length() > maxDisplayWidth) {
                truncatedLine.resize(maxDisplayWidth);
            }
            mvwprintw(popupWin, currentLineY++, textStartX, "%s", truncatedLine.c_str());
        }
    }

    // --- Display Continue Prompt ---
    int promptY = popupHeight - 1 - 1;
    int promptX = std::max(horizontalPadding + 1,
                           (popupWidth - static_cast<int>(continuePrompt.length())) / 2);
    mvwprintw(popupWin, promptY, promptX, "%s", continuePrompt.c_str());
    wrefresh(popupWin);
    wgetch(popupWin);
    delwin(popupWin);

    // Touch the main screen and refresh to redraw the underlying game state cleanly
    touchwin(stdscr);
    refresh();
}

bool Gameplay::displayQuitOptions() {
    // Padding
    const int horizontalPadding = 3;
    const int verticalPadding = 1;

    // --- Calculate window dimensions ---
    std::string title = "Quit Game";
    std::string message = "Are you sure you want to quit?";
    std::string option1 = "Yes";
    std::string option2 = "No";

    int maxTextLength =
        std::max({title.length(), message.length(), option1.length() + option2.length() + 4});

    int popupHeight = 7;  // Title, message, options, borders
    int popupWidth = 1 + horizontalPadding + maxTextLength + horizontalPadding + 1;

    // Ensure dimensions are valid
    popupWidth = std::max(30, popupWidth);

    int popupY = (height - popupHeight) / 2;
    int popupX = (width - popupWidth) / 2;

    WINDOW* popupWin = newwin(popupHeight, popupWidth, popupY, popupX);
    keypad(popupWin, TRUE);
    box(popupWin, 0, 0);

    // Record time when the pause started
    auto pauseStartTime = std::chrono::steady_clock::now();

    // --- Display Title (Centered) ---
    int titleX = (popupWidth - static_cast<int>(title.length())) / 2;
    wattron(popupWin, A_BOLD);
    mvwprintw(popupWin, 1, titleX, "%s", title.c_str());
    wattroff(popupWin, A_BOLD);

    // Display Message
    int messageX = (popupWidth - static_cast<int>(message.length())) / 2;
    mvwprintw(popupWin, 3, messageX, "%s", message.c_str());

    // Options
    bool selectedYes = true;  // Default to Yes
    int optionsY = 5;

    // Input loop for handling selection
    nodelay(popupWin, FALSE);  // Wait for input in this window

    bool madeSelection = false;
    while (!madeSelection) {
        // Calculate positions for yes/no
        int totalOptionsWidth = option1.length() + option2.length() + 4;
        int optionsStartX = (popupWidth - totalOptionsWidth) / 2;

        int yesX = optionsStartX;
        int noX = yesX + option1.length() + 4;

        // Draw Yes option
        if (selectedYes)
            wattron(popupWin, A_REVERSE);
        mvwprintw(popupWin, optionsY, yesX, "%s", option1.c_str());
        if (selectedYes)
            wattroff(popupWin, A_REVERSE);

        // Draw No option
        if (!selectedYes)
            wattron(popupWin, A_REVERSE);
        mvwprintw(popupWin, optionsY, noX, "%s", option2.c_str());
        if (!selectedYes)
            wattroff(popupWin, A_REVERSE);

        wrefresh(popupWin);

        // Get input
        int ch = wgetch(popupWin);
        switch (ch) {
            case KEY_LEFT:
                selectedYes = true;
                break;
            case KEY_RIGHT:
                selectedYes = false;
                break;
            case '\n':  // Enter
            case KEY_ENTER:
                madeSelection = true;
                break;
            case 27:  // ESC
                selectedYes = false;
                madeSelection = true;
                break;
        }
    }

    nodelay(popupWin, TRUE);  // Restore non-blocking
    delwin(popupWin);

    // Adjust the startTime by the duration spent in this dialog
    auto pauseEndTime = std::chrono::steady_clock::now();
    auto pauseDuration = pauseEndTime - pauseStartTime;
    startTime += pauseDuration;

    // Redraw the screen
    touchwin(stdscr);
    refresh();

    return selectedYes;
}

void Gameplay::saveGameState() {
    std::ofstream saveFile("savegame.txt");
    if (saveFile.is_open()) {
        // Save essential game variables
        saveFile << difficultyHighlight << std::endl;
        saveFile << roundNumber << std::endl;
        saveFile << totalScore << std::endl;
        saveFile << lastRoundStepScore << std::endl;
        saveFile << lastRoundTimeScore << std::endl;
        saveFile << currentStamina << std::endl;
        saveFile << maxStamina << std::endl;
        saveFile.close();
        addHistoryMessage("Game saved successfully.");
    } else {
        addHistoryMessage("Failed to save game.");
    }
}

void Gameplay::loadGameState() {
    std::ifstream saveFile("savegame.txt");
    if (saveFile.is_open()) {
        int savedDifficulty;
        saveFile >> savedDifficulty;

        // Only load if difficulty matches or set proper difficulty
        difficultyHighlight = savedDifficulty;

        saveFile >> roundNumber;
        saveFile >> totalScore;
        saveFile >> lastRoundStepScore;
        saveFile >> lastRoundTimeScore;
        saveFile >> currentStamina;
        saveFile >> maxStamina;

        staminaAtRoundStart = currentStamina;
        saveFile.close();

        // Update difficulty-dependent settings
        switch (difficultyHighlight) {
            case 0:
                diff_str = "Easy";
                map_size = 15;  // Adjust in future
                num_obs = 5;
                num_pkg = 3;
                break;
            case 1:  // Medium
                diff_str = "Medium";
                map_size = 20;  // Same above
                num_obs = 6;
                num_pkg = 4;
                break;
            case 2:  // Hard
                diff_str = "Hard";
                map_size = 25;
                num_obs = 7;
                num_pkg = 5;
                break;
            default:
                diff_str = "Unknown";
                map_size = 15;
                num_obs = 5;
                num_pkg = 3;
                break;
        }

        addHistoryMessage("Game loaded successfully.");
        addHistoryMessage("Continuing from Round " + std::to_string(roundNumber));
    } else {
        // If loading fails, start a new game
        addHistoryMessage("No saved game found. Starting new game.");
        roundNumber = 1;
        currentStamina = 200;
        maxStamina = 200;
        staminaAtRoundStart = 200;
        totalScore = 0;
        lastRoundStepScore = 0;
        lastRoundTimeScore = 0;
    }
}