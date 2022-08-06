#ifndef SIK2_SERVER_H
#define SIK2_SERVER_H

#define MAX_USERS 25
#define MAX_UDP_MSG_LEN 548

#include "player.h"
#include "viewer.h"
#include "event.h"

#include <time.h>
#include <getopt.h>
#include <cctype>
#include <climits>
#include <endian.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>
#include <map>
#include <stdexcept>
#include <optional>
#include <algorithm>

class Server {
public:
    /** Construct server object with default values. */
    Server() : port(2021), seed(time(NULL)),
               turning_speed(6), rounds_per_sec(50),
               width(640), height(480), in_game(false), users(), worms(),
               events(), timer(), board(width, height) {
        timer.fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
        timer.events = POLLIN;
        timer.revents = 0;
    }

    /** Parse main arguments and
     * change server's parameters with a help of getopt function. */
    void set_parameters(const int argc, char *const argv[]);

    /** Print actual parameters */
    void test_parameters() noexcept;

    /** Set timers' specifications. Open socket and start listening.
     * Handle the game - Receive from players, update game state and send messages back.
     */
    void run();

private:
    int32_t port;
    uint64_t seed;
    int32_t turning_speed;
    int32_t rounds_per_sec;
    int32_t width;
    int32_t height;
    int32_t sock;
    bool in_game;
    uint32_t game_id;
    std::map <std::string, std::shared_ptr<User>> users;
    std::vector <std::shared_ptr<Worm>> worms;
    std::vector <Event> events;
    struct pollfd timer;
    struct itimerspec user_timer_spec;
    struct itimerspec game_timer_spec;
    Board board;

    /** Validates if parameter is a number between lo and hi, otherwise throws an exception. */
    int64_t validate_parameter(const std::string &param, const int64_t lo, const int64_t hi);

    /** Sets user timeout. */
    void set_user_timer_spec() noexcept;

    /** Sets game timeout. */
    void set_game_timer_spec() noexcept;

    /** Returns random number. */
    uint32_t rand() noexcept;

    /** Creates socket and start listening for udp datagram. */
    void listen();

    /** Read datagram from client, convert bytes (big endian to host),
     * validate player name and message length.
     * If datagram is correct proceed to handle_msg.*/
    void read_datagram();

    /** Checks if user is already connected. Send response message.
     * Reset user's timeout.
     * If user is new, add him to game. */
    void handle_msg(const client_server_msg &msg, const struct sockaddr_in6 &addr);

    /** Returns false when player's name contains banned characters. */
    bool validate_player_name(const client_server_msg &msg, const int32_t len) noexcept;

    /** Returns player's name or std::nullopt if one is only a viewer. */
    const std::optional <std::string> get_username(const client_server_msg &msg);

    /** Gets client id out of address.
     * Client id is a string of 3 - ip address, separator, port. */
    const std::string get_client_id(const struct sockaddr_in6 &addr);

    /** Adds new user - player or viewer based on one's name. */
    void add_new_user(const client_server_msg &msg, const std::string &id, const struct sockaddr_in6 &addr);

    /** Deletes user from server.*/
    void disconnect_user(const std::string &id);

    /** Checks if every user sent datagram in the last 2 seconds,
     * otherwise disconnect inactive users.  */
    void check_players_timeout();

    /** Start game round timer. */
    void set_round_time();

    /** Check if the next turn has to be done
     * and proceed to make_turn() */
    void turn_timeout();

    /** Move all alive worms by 1 in their directions. */
    void make_turn();

    /** Check if players are ready and proceed to start_game() */
    void prepare_game();

    /** Generate worms start positions */
    void start_game();

    /** Create worms, connect each one to its user
     * and sort them in alphabetical order. */
    void create_worms();

    /** Generate new game event, save event
     * and broadcast it to all users. */
    void new_game_event();

    /** Generate pixel event, save event
     * and broadcast it to all users. */
    void pixel_event(uint8_t num, uint32_t x, uint32_t y);

    /** Generate player eliminated event, save event
     * and broadcast it to all users. */
    void player_eliminated_event(uint8_t num);

    /** Generate game over event, save event
     * and broadcast it to all users. */
    void game_over_event();

    /** Send the newest event to all users. */
    void broadcast_new_event();

    /** Response on client message if there is the right event. */
    void response(const uint32_t next_expected_event_no, const struct sockaddr_in6 &addr);

    /** Send GAME_OVER event, delete worms,
     * reset players ready flag. */
    void game_over();

};


#endif //SIK2_SERVER_H
