#include "../include/gameplay.h"

#include <ncurses.h>

#include <chrono>  // For timing
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>  // Not used for now, but might be needed later
#include <utility>
#include <algorithm>
#include <set>

#include "../include/game.h"

// Map initialization helper functions
void Gameplay::initializeMap() {
    mapGrid.assign(map_size, std::string(map_size, '.'));

    // Top and Bottom borders
    for (int x = 0; x < map_size; ++x) {
        mapGrid[0][x] = '-';
        mapGrid[map_size - 1][x] = '-';
        // I have gave up using window drawing here, since it is not working well
        // The playarea is having a hardtime aligning with the window borders
        // Character "-" has space in between when displaying multiple of it in a row,
        // suggest fiding a better character to use for borders
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
    // Or you can use special characters for each of the four corners, 
    // if Linux terminal can support it


    // Seed
    srand(time(0));

    // Reset Delivered Count for New Round
    packagesDelivered = 0;

    // Define Player Start and Exit Locations
    playerY = map_size / 2;
    playerX = 1;
    exitY = map_size / 2;
    exitX = map_size - 2;
    mapGrid[exitY][exitX] = 'Q'; // Place exit marker

    // Determine package locations (protected)
    std::vector<std::pair<int, int>> packageLocations;

    // Place Obstacles
    // Currently placeholder, as the obstacle placement is being scattered on the map, not as intended.
    // Use for previewing

    int obstaclesPlaced = 0;
    while (obstaclesPlaced < num_obs) {
        int y = (rand() % (map_size - 2)) + 1;
        int x = (rand() % (map_size - 2)) + 1;
        // Ensure not placing on player start or exit
        if (mapGrid[y][x] == '.' && !(y == playerY && x == playerX) && !(y == exitY && x == exitX)) {
            mapGrid[y][x] = '#';
            obstaclesPlaced++;
        }
    }

    // Place Packages
    // Also placeholder. Refine placement later.
    // Use for previewing


    int packagesToPlace = num_pkg;
    int packagesPlaced = 0;
    while (packagesPlaced < packagesToPlace) {
         int y = (rand() % (map_size - 2)) + 1;
         int x = (rand() % (map_size - 2)) + 1;
         if (mapGrid[y][x] == '.') {
             mapGrid[y][x] = 'O';
             packagesPlaced++;
         }
    }

    // Place Supplies / Speed Bumps
    // Pass this to you Hyper-aceX :)
}

// Constructor initializes windows based on difficulty
Gameplay::Gameplay(const int& difficultyHighlight, GameState& current_state)
    : difficultyHighlight(difficultyHighlight),
      current_state(current_state),
      map_size(0),
      num_obs(0),
      num_pkg(0),
      roundNumber(1),
      currentStamina(200),
      maxStamina(200),
      currentPackageIndex(-1),
      packagesDelivered(0),
      playerY(0), playerX(0),
      exitY(0), exitX(0)
{
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
    int legendHeight = std::max(minLegendHeight, desiredLegendHeight); // Ensure minimum height
    legendHeight = std::min(height - bottomPanelHeight, legendHeight); // Don't exceed available space above bottom bar

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
        case 'w': // Move Up
        case KEY_UP: // Also handle arrow key
            nextY--;
            moved = true;
            break;
        case 's': // Move Down
        case KEY_DOWN:
            nextY++;
            moved = true;
            break;
        case 'a': // Move Left
        case KEY_LEFT:
            nextX--;
            moved = true;
            break;
        case 'd': // Move Right
        case KEY_RIGHT:
            nextX++;
            moved = true;
            break;

        // --- Exit Interaction ---
        case '\n':      // Enter key
        case KEY_ENTER: // Ncurses specific Enter key
            if (playerY == exitY && playerX == exitX) {
                // Check if all packages are delivered
                if (packagesDelivered >= num_pkg) {
                    roundNumber++;
                    addHistoryMessage("Level Complete! Proceeding to Round " + std::to_string(roundNumber) + "...");
                    initializeMap(); // Regenerate map, reset player pos, reset delivered count
                    // Considering adding a screen pause here for user to read the message

                    // Reset package holding status
                    std::fill(hasPackage.begin(), hasPackage.end(), false);
                    currentPackageIndex = -1;

                    // Add score bonus, reset timer bonus, etc. here later

                } else {
                    addHistoryMessage("Cannot exit yet! Deliver all packages first. ("
                                      + std::to_string(packagesDelivered) + "/"
                                      + std::to_string(num_pkg) + " delivered)");
                }
            } else {
                // Optional: Message if Enter pressed not at exit
                // addHistoryMessage("Press Enter only at the exit 'Q'.");
            }
            break;

        // --- Package Selection ---
        case '1': if (num_pkg >= 1) currentPackageIndex = 0; addHistoryMessage("Selected package 1."); break;
        case '2': if (num_pkg >= 2) currentPackageIndex = 1; addHistoryMessage("Selected package 2."); break;
        case '3': if (num_pkg >= 3) currentPackageIndex = 2; addHistoryMessage("Selected package 3."); break;
        case '4': if (num_pkg >= 4) currentPackageIndex = 3; addHistoryMessage("Selected package 4."); break;
        case '5': if (num_pkg >= 5) currentPackageIndex = 4; addHistoryMessage("Selected package 5."); break;

        // --- Package Pickup/Drop ---
        // No pickup / drop implemented yet.
        case 'q':
             // TODO: Implement actual pickup logic
             addHistoryMessage("Pressed Q (Pickup - Placeholder)");
             break;
        case 'e':
             // TODO: Implement actual drop logic (at destination)
             // For now use 'e' to simulate delivering the current package
             if (currentPackageIndex != -1 && hasPackage[currentPackageIndex]) {
                 int pkgIdx = currentPackageIndex;
                 addHistoryMessage("Delivered package " + std::to_string(pkgIdx + 1) + " (Placeholder).");
                 hasPackage[pkgIdx] = false;
                 packagesDelivered++;

                 // Find next held package or set to -1
                 currentPackageIndex = -1;
                 for(int i = 0; i < num_pkg; ++i) {
                     if(hasPackage[i]) {
                         currentPackageIndex = i;
                         break;
                     }
                 }
             } else {
                 addHistoryMessage("No package selected to deliver.");
             }
             break;

        case 27: // ESC
            addHistoryMessage("Exiting to main menu...");
            current_state = GameState::MAIN_MENU;
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
                // Check Stamina
                // I think in the original game design, goal is to end the game when stamina is 0
                if (currentStamina > 0) {
                    int oldStamina = currentStamina;
                    currentStamina--;
                    currentStamina = std::max(0, currentStamina);

                    // Update Player Position
                    playerY = nextY;
                    playerX = nextX;

                    addHistoryMessage("Moved. Stamina: " + std::to_string(oldStamina) + " -> " + std::to_string(currentStamina));

                    // TODO: Check for landing on items
                    // if (mapGrid[playerY][playerX] == 'Q') { [win condition here] }

                } else {
                    addHistoryMessage("Cannot move! Out of stamina.");
                }
            } else {
                addHistoryMessage("Cannot move! Blocked by obstacle.");
            }
        } else {
            addHistoryMessage("Cannot move! Hit the border.");
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
        init_pair(1, COLOR_YELLOW, COLOR_BLACK);
        init_pair(2, COLOR_CYAN, COLOR_BLACK);
        init_pair(3, COLOR_BLUE, COLOR_BLACK);
        // 3 used for dots indicating empty spaces.
        // Alternatively, try grey if available, if blue does not look great: 
        // init_pair(3, COLOR_BLACK + 8, COLOR_BLACK);
        // This may not work on our terminals so suggest testing on CS servers.
    }

    addHistoryMessage("Game Started. Round " + std::to_string(roundNumber));
    startTime = std::chrono::steady_clock::now();

    // Example: Simulate picking up the first package initially
    if (num_pkg > 0) {
        hasPackage[0] = true;
        currentPackageIndex = 0;  // Select the first package
        addHistoryMessage("Picked up package 1.");
    }

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
             break;
        }
        napms(50);
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
            if (winY >= 0 && winY < maxY && winX >= 0 && winX < maxX -1)
            {
                 char displayChar = mapGrid[y][x];
                 bool applyColor = false;

                 // Apply color for '.'
                 if (displayChar == '.') {
                     wattron(mapWin, COLOR_PAIR(3));
                     applyColor = true;
                 }
                 // Add else if blocks here for other character colors 
                 // e.g., obstacles, packages

                 mvwaddch(mapWin, winY, winX, displayChar);

                 if (applyColor) {
                     wattroff(mapWin, COLOR_PAIR(3));
                 }
            }
        }
    }

    // Draw Player
    int playerWinY = playerY;
    int playerWinX = playerX * 2;

    // Check if player position is within the window bounds
     if (playerWinY >= 0 && playerWinY < maxY &&
         playerWinX >= 0 && playerWinX < maxX -1)
     {
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
    int row = 1;
    int col = 2;
    mvwprintw(timeWin, row++, col, "Elapsed: %s", timeStr.c_str());
    row++;  // Add a blank line
    mvwprintw(timeWin, row++, col, "Time Bonus:");
    mvwprintw(timeWin, row++, col,
              "  (Not Implemented)");  // Placeholder, will be implemented later

    wnoutrefresh(timeWin);
}

void Gameplay::displayLegend() {
    werase(legendWin);
    box(legendWin, 0, 0);

    // Display Round Number
    std::string roundText = "Round " + std::to_string(roundNumber);
    wattron(legendWin, A_BOLD);
    mvwprintw(legendWin, 0, 2, " %s ", roundText.c_str());
    wattroff(legendWin, A_BOLD);

    // --- Starting row for content ---
    int row = 2;
    int col = 2;

    // --- Legend Content ---
    mvwprintw(legendWin, row++, col, "--- Legend ---");
    mvwprintw(legendWin, row++, col, " @: Player");
    mvwprintw(legendWin, row++, col, " #: Obstacle");
    mvwprintw(legendWin, row++, col, " ~: Speed Bump");
    mvwprintw(legendWin, row++, col, " $: Supply");
    mvwprintw(legendWin, row++, col, " O: Package");
    mvwprintw(legendWin, row++, col, " Q: Exit");
    row++;

    // --- Movement Controls ---
    mvwprintw(legendWin, row++, col, "--- Movement ---");
    mvwprintw(legendWin, row++, col, "   W: Move Up");
    mvwprintw(legendWin, row++, col, "   S: Move Down");
    mvwprintw(legendWin, row++, col, "   A: Move Left");
    mvwprintw(legendWin, row++, col, "   D: Move Right");
    row++;

    // --- Package Controls ---
    mvwprintw(legendWin, row++, col, "--- Package ---");
    mvwprintw(legendWin, row++, col, "   Q: Pick Up");
    mvwprintw(legendWin, row++, col, "   E: Drop current package");
    mvwprintw(legendWin, row++, col, " 1-5: Switch current package");  // Simplified
    row++;

    // --- Game Controls ---
    mvwprintw(legendWin, row++, col, "---- Game ----");
    mvwprintw(legendWin, row++, col, " Enter: Next Level (at Q,all pkgs delivered)");
    mvwprintw(legendWin, row++, col, "   ESC: Pause");
    mvwprintw(legendWin, row++, col, " Hold ESC: Quit");

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
    mvwprintw(packageWin, 0, 2, title);

    // --- Package Slot Display ---
    const char* emptySlot = "_";

    // On windows these circled characters are not working.
    // So we will use numbers instead.
    // I'm not sure if Linux supports these characters, but let's try.
    // If not, we can use numbers as well.
#ifdef _WIN32
    const char* packageChars[] = {"1", "2", "3", "4", "5"};
#else
    const char* packageChars[] = {"①", "②", "③", "④", "⑤"};
#endif

    int startX = 2;  // Starting column inside the box
    int currentX = startX;
    int yPos = 1;  // Row inside the box

    for (int i = 0; i < num_pkg; ++i) {
        const char* displayChar = emptySlot;
        if (hasPackage[i]) {
            if (i < 5) {
                displayChar = packageChars[i];
            } else {
                displayChar = "?";  // Fallback if more than 5 packages somehow
            }
        }

        // Check for highlight
        if (i == currentPackageIndex) {
            wattron(packageWin, A_REVERSE);  // Highlight the current package
        }

        // Print the character - mvwaddstr works for both single-byte and multi-byte chars
        mvwaddstr(packageWin, yPos, currentX, displayChar);

        // Turn off highlight if it was on
        if (i == currentPackageIndex) {
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
