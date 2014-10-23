hexchat-indicator
=================

Ubuntu messaging menu indicator for Hexchat IRC client

It's a port of xchat-indicator, you can find it on [Launchpad](https://launchpad.net/xchat-indicator)

installation
-----------------

Should work in Ubuntu 14.04

    sudo apt-get install libindicate-dev libunity-dev libmessaging-menu-dev automake
    cd hexchat-indicator
    ./configure
    make
    cp ./.libs/indicator.so $HOME/.config/hexchat/addons

