# **Worms game** - MIMUW assignment - network programming class

This is a simple network game, in which the last living worm wins!

Written in C++. Client and Server communicate with UDP, Client and GUI communicate with TCP. 

Unfortunately, works only on linux :(

**Note:** I am not the author of GUI - it was provided by University of Warsaw.

# how to run
- build Server, Client and GUI via **Makefile** 

- run GUI

- run Server:

  ./screen-worms-server [-p n] [-s n] [-t n] [-v n] [-w n] [-h n]

    * `-p n` – port number (default `2021`)
    * `-s n` – seed (default `time(NULL)`)
    * `-t n` – turning speed (default `6`)
    * `-v n` – rounds per second, game speed (default `50`)
    * `-w n` – board width (default `640`)
    * `-h n` – board heigth (default `480`)

- run Client :
    
    ./screen-worms-client game_server [-n player_name] [-p n] [-i gui_server] [-r n]

  * `game_server` – IPv4 or IPv6 or server name
  * `-n player_name` 
  * `-p n` – server port (ddefault `2021`)
  * `-i gui_server` – IPv4 or IPv4 or server name (default `localhost`)
  * `-r n` – gui server port  (default `20210`)


