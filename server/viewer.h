#ifndef SIK2_VIEWER_H
#define SIK2_VIEWER_H

#include "user.h"

/** Viewer is basically a user that does not change the game itself.
 * Only gets information about the game events,
 * thus all its functions are idle. */
class Viewer : public User {
public:
    Viewer(const uint64_t sess_id, const struct sockaddr_in6 &a) : User(sess_id, a) {
        std::cout << "Connected new viewer" << std::endl;
    }

    void move(const client_server_msg &);

    void set_ready(const client_server_msg &);

    void set_ready(const bool);

    int32_t is_ready();

    const std::string get_name();

    void set_worm(const std::shared_ptr <Worm> &);

    int32_t get_last_turn();

};


#endif //SIK2_VIEWER_H
