#include "user.h"

void User::set_timer(const struct itimerspec &spec_timer) {
    timerfd_settime(timer.fd, 0, &spec_timer, NULL);
    timer.revents = 0;
}

int32_t User::is_expired() {
    int32_t res = poll(&timer, 1, 0);
    if (res && timer.revents & POLLIN) {
        return true;
    }
    return false;
}

void User::close_fd() {
    if (close(timer.fd) == -1) {
        std::cerr << "timer closing fail" << std::endl;
    }
}

uint64_t User::get_session_id() noexcept {
    return session_id;
}

const struct sockaddr_in6 &User::get_addr() {
    return addr;
}
