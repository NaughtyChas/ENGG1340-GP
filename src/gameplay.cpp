#include "../include/gameplay.h"

#include <ncurses.h>

#include <chrono>  // For timing
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>  // Not used for now, but might be needed later

#include "../include/game.h"

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
      currentPackageIndex(-1)  // Start with no package selected
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
    int legendHeight = std::min(height - bottomPanelHeight, height / 2);
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

void Gameplay::run() {
    // Clear remnants from mainWindow
    clear();
    refresh();

    // Initialize colors if not done elsewhere (ensure start_color() was called)
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_YELLOW, COLOR_BLACK);
        init_pair(2, COLOR_CYAN, COLOR_BLACK);
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

        // --- Basic Input Handling ---
        // Not that implemented yet.
        bool staminaChanged = false;
        switch (ch) {
            case 27:
                // TODO: might draw a new window to confirm exit
                addHistoryMessage("Exiting to main menu...");  // Example message
                current_state = GameState::MAIN_MENU;
                return;
            case KEY_RESIZE:
                addHistoryMessage("Terminal resized.");  // Example message
                clear();
                refresh();
                break;
            case 'a':  // in case where the stamina is decreased
                if (currentStamina > 0) {
                    int oldStamina = currentStamina;
                    currentStamina -= 10;  // Decrease by 10 for now, press a for testing
                    currentStamina = std::max(0, currentStamina);
                    staminaChanged = true;
                    addHistoryMessage("Stamina decreased: " + std::to_string(oldStamina) + " -> " +
                                      std::to_string(currentStamina));
                } else {
                    addHistoryMessage("Stamina already empty!");
                }
                break;
            case 'd':  // Increase stamina
                if (currentStamina < maxStamina) {
                    int oldStamina = currentStamina;
                    currentStamina += 10;  // Increase by 10 for now, also for testing
                    currentStamina = std::min(maxStamina, currentStamina);
                    staminaChanged = true;
                    addHistoryMessage("Stamina increased: " + std::to_string(oldStamina) + " -> " +
                                      std::to_string(currentStamina));  // Example message
                } else {
                    addHistoryMessage("Stamina already full!");
                }
                break;

            // --- Package Selection Test Input ---
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
            // Example Pickup/Drop Test. Fill in with actual game logic later.
            case 'q':  // Simulate picking up next available package
                for (int i = 0; i < num_pkg; ++i) {
                    if (!hasPackage[i]) {
                        hasPackage[i] = true;
                        currentPackageIndex = i;
                        addHistoryMessage("Picked up package " + std::to_string(i + 1) + ".");
                        break;
                    }
                }
                break;
            case 'e':  // Simulate dropping current package
                if (currentPackageIndex != -1 && hasPackage[currentPackageIndex]) {
                    hasPackage[currentPackageIndex] = false;
                    addHistoryMessage("Dropped package " + std::to_string(currentPackageIndex + 1) +
                                      ".");
                    currentPackageIndex = -1;            // Deselect after dropping
                    for (int i = 0; i < num_pkg; ++i) {  // Select first available, if any
                        if (hasPackage[i]) {
                            currentPackageIndex = i;
                            break;
                        }
                    }
                }
                break;

            case ERR:  // No input
                break;
                // Other game input handling should be implemented here...
                // W, A, S, D for movement, Q for package handling, etc.
        }

        // --- Game Logic Update ---
        // Wrote these comments to remind you to add game logic updates here.
        // For example, if stamina changed, you might want to check if the player can move or
        // perform actions. If stamina changed, the next loop iteration will redraw the bar. Add
        // other game logic updates here (e.g., move player, check collisions)

        napms(100);
    }
}

// Display functions
void Gameplay::displayMap() {
    werase(mapWin);
    box(mapWin, 0, 0);
    mvwprintw(mapWin, 0, 2, diff_str.c_str());
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
    mvwprintw(legendWin, row++, col, " ESC: Pause");
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
        maxWidth = std::max(0, maxWidth); // Ensure maxWidth is not negative
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
