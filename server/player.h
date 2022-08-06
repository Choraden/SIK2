#ifndef SIK2_PLAYER_H
#define SIK2_PLAYER_H

#include "user.h"

#include <memory>

class Player : public User {
public:
    Player(const uint64_t sess_id, const struct sockaddr_in6 &a,
           const std::string &player_name, int32_t lt) : User(sess_id, a),
                                                         name(player_name),
                                                         worm(NULL), ready(false),
                                                         last_turn(lt) {
        ready = lt == 1 || lt == 2;
        std::cout << "Connected new player " << name << std::endl;
        if (ready == 1)
            std::cout << name << " is ready!" << std::endl;
    }

    /** Change turn direction of player's worm, and set last turn */
    void move(const client_server_msg &msg);

    void set_ready(const client_server_msg &msg);

    void set_ready(const bool rdy);

    int32_t is_ready();

    const std::string get_name();

    void set_worm(const std::shared_ptr <Worm> &ptr);

    int32_t get_last_turn();

private:
    std::string name;
    std::shared_ptr <Worm> worm;
    bool ready;
    int32_t last_turn;
};


#endif //SIK2_PLAYER_H
