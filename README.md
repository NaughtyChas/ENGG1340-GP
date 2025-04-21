# ENGG1340-GP
Game built for ENGG1340 group project.
## Compile & Run

### For Linux user:

1. Open the terminal in the game folder.
2. To run the game directly, please run `./start.sh`, this will compile and run the game.
   To only compile the game, please run `make all`, then the executable file `./bin/FPS_Simulator` will be generated.
3. Run `make clean` to delete all the object files and binary files generated.
4. Run `make tar` to pack the whole project (excluding the object and binary files) into an archive file.

- **Important**: Make sure the terminal size is at least 200 * 75
- **For those playing on HKU CS server**: Due to the unstable performance and the latency issue, the game sometimes lags when the CS server is under high load. We recommend you to play the game on your own PC or VM for a better experience.

### For Windows user:

Although the game is adapted for both Linux and Windows, we still recommend you to play it on **Linux**. Due to some unknown cache refreshing mechanism issues, the flushing rate on Windows terminal is far slower than that on Linux terminal.