#ifndef SIK2_USER_H
#define SIK2_USER_H

#include "../shared/client_server_msg.h"
#include "worm.h"

#include <iostream>
#include <sys/timerfd.h>
#include <sys/poll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <memory>

#define READY 1

class User {
public:
    User(const uint64_t sess_id, const struct sockaddr_in6 &a) : session_id(sess_id), timer(), addr(a) {
        timer.fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
        timer.events = POLLIN;
        timer.revents = 0;
    }

    virtual ~User() = default;

    virtual void move(const client_server_msg &msg) = 0;

    virtual void set_ready(const client_server_msg &msg) = 0;

    virtual void set_ready(const bool rdy) = 0;

    virtual int32_t is_ready() = 0;

    virtual const std::string get_name() = 0;

    virtual void set_worm(const std::shared_ptr <Worm> &ptr) = 0;

    virtual int32_t get_last_turn() = 0;

    void set_timer(const struct itimerspec &spec_timer);

    /** Return if user is inactive (hasn't sent datagram in the last 2 seconds). */
    int32_t is_expired();

    /** Destroy user's timer. */
    void close_fd();

    uint64_t get_session_id() noexcept;

    const struct sockaddr_in6 &get_addr();


protected:
    uint64_t session_id;
    struct pollfd timer;
    struct sockaddr_in6 addr;

};


#endif //SIK2_USER_H
