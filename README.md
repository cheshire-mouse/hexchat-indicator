hexchat-indicator
=================

Ubuntu messaging menu indicator for Hexchat IRC client

installation
-----------------

Should work in Ubuntu 14.04

    sudo apt-get install libindicate-dev libunity-dev
    cd hexchat-indicator
    ./configure
    make
    cp ./.libs/indicator.so $HOME/.config/hexchat/addons

