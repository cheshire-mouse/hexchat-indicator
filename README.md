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

Known problems
-----------------

####Clicking on messaging menu indicator starts a new instance of the application

Strictly speaking, it's not hexchat-indicator problem. However, there is a workaround that can help.

* copy **/usr/share/applications/hexchat.desktop** to **/usr/local/share/applications/hexchat.desktop** (create /usr/local/share/applications folder if it doesn't exist)
* open **/usr/local/share/applications/hexchat.desktop** with a text editor
* find the line "Exec=...." in [Desktop Entry] section
* comment it and add a new one 
```
#Exec=hexchat %U
Exec=sh -c "flock -n -x /var/run/lock/hexchat.lock -c hexchat || hexchat --existing -c 'gui show'"
```
* reboot or logout/login

Now clicking on the indicator should show existing hexchat window instead of starting a new application
