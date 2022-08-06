#ifndef SIK2_CLIENT_H
#define SIK2_CLIENT_H

#define BYTE 8
#define STRAIGHT 0
#define RIGHT 1
#define LEFT 2
#define DWORD 4
#define QWORD 8
#define NEW_GAME 0
#define PIXEL 1
#define PLAYER_ELIMINATED 2
#define GAME_OVER 3


#include "../shared/client_server_msg.h"

#include <iostream>
#include <cstdint>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/timerfd.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <optional>
#include <vector>
#include <endian.h>
#include <netinet/tcp.h>

class Client {
public:
    Client() : name(), server_port("2021"), gui_server("localhost"),
               gui_port("20210"), game_id(0), last_turn(0),
               next_expected_event_no(0),
               session_id(time(NULL)), timer(), gui_comm{""} {
        timer.fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
        timer.events = POLLIN;
        timer.revents = 0;
    }

    /** Parse main arguments and
     * change client's parameters with a help of getopt function. */
    void set_parameters(const int argc, char *argv[]);

    /** Print actual parameters */
    void test_parameters();

    /** Set timer specifications, launch it.
     * Connect to server and gui.
     * Start playing! - Receive from gui, pass it to game and the other way around.
     */
    void run();

private:
    std::string name;
    std::string server_name;
    std::string server_port;
    std::string gui_server;
    std::string gui_port;
    int32_t server_sock;
    int32_t gui_sock;
    uint32_t game_id;
    uint8_t last_turn; /// The newest direction from gui - straight, right or left.
    uint32_t next_expected_event_no;
    std::vector <std::string> player_name; /// Store names of all players in current game.
    uint32_t width;
    uint32_t height;
    uint64_t session_id;
    struct pollfd timer;
    struct itimerspec timer_spec;
    std::string gui_comm; /// Buffer for parsing gui messages.


    /** Validates if name is not too long, and contains only allowed characters. */
    bool validate_player_name(const std::string &player_name);

    /** Connect to address - gui_server, on port - gui_port. TCP */
    void connect_to_gui();

    /** Connect to address - server_name, on port server_port. UDP */
    void open_server_sock();

    /** Validates if parameter is a number between lo and hi, otherwise throws an exception. */
    int64_t validate_parameter(const std::string &param, const int64_t lo, const int64_t hi);

    /** Receive message from gui and add its content to buffer gui_comm,
     * for later processing. */
    void receive_from_gui();

    /** Send info about events received from game server. */
    void send_to_gui(std::string &msg);

    /** Parse gui message, process the right ones, ignore wrong. */
    void parse_gui_msg(const std::string &msg);

    /** Process gui message and set the new value of direction
     * in last_turn variable - straight, right or left. */
    void move(const std::string &comm);

    /** Send udp datagram to game server containing session_id,
     * next_expected_event_number, last_turn and player's name. */
    void send_to_game();

    /** Receive upd datagram from game server,
     * containing information about game events. */
    void receive_from_game();

    /** Set timer to counts 30ms down for the next message to be sent to game. */
    void set_timer_spec();

    void launch_timer();

    /** Validate received datagram with crc algorithm. Ignore wrong ones. */
    bool check_crc(uint8_t *buff, size_t offset, uint32_t len);

    /** Deduce type of event and validate correctness of datagram.
     * Proceed to specialized processing of events
     * - it is new game, pixel, elimination or game over.
     * @param buff - binary data of event
     * @param offset - position in buffer */
    bool process_event(uint8_t *buff, size_t &offset, uint32_t game_id_);

    /** Set width and height of the new game, save players names and indexes.
     * proceed to sending new game event to gui. */
    void process_new_game_event(uint8_t *buff, size_t &offset, uint32_t len);

    /** Get player number, position (x, y), validate data.
     * Proceed to sending pixel event to gui. */
    void process_pixel_event(uint8_t *buff, size_t &offset, uint32_t len);

    /** Get player number, validate it.
     * Proceed to sending elimination event to gui. */
    void process_player_eliminated_event(uint8_t *buff, size_t &offset, uint32_t len);

    /** Save new players at the beginning of the game. */
    void get_new_players(uint8_t *buff, size_t &offset, uint32_t len);

    /** Get four bytes from received datagram - reverse it from big endian to host.
     * Shift offset by four. */
    uint32_t get_dword(uint8_t *buff, size_t &offset);

    /** Get eight bytes from received datagram - reverse it from big endian to host.
     * Shift offset by eight. */
    uint64_t get_qword(uint8_t *buff, size_t &offset);

    /** Get byte from received datagram.
     * Shift offset by one. */
    uint8_t get_byte(uint8_t *buff, size_t &offset);

    /** Construct new game event and send it to gui. */
    void send_new_game_event();

    /** Construct pixel event and send it to gui. */
    void send_pixel_event(const uint8_t n, const uint32_t x, const uint32_t y);

    /** Construct elimination event and send it to gui. */
    void send_player_eliminated_event(const uint8_t n);

};


#endif //SIK2_CLIENT_H
