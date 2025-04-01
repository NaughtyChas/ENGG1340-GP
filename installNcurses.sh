#!/bin/bash

if [ ! -d ncurses ]; then
    echo "Downloading ncurses..."
    git clone https://github.com/mirror/ncurses.git
    echo "Downloaded successfully!"
    
    cd ncurses
    mkdir -p ~/local
    ./configure --prefix=~/local
fi

echo "Installing ncurses..."
cd ncurses
make -j
make install
echo "Installed successfully!"
echo "Done!"