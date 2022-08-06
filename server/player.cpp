#include "player.h"

void Player::move(const client_server_msg &msg) {
    if (worm != NULL) {
        worm->set_turn(msg.turn_direction);
    }
    last_turn = msg.turn_direction;
}

void Player::set_ready(const client_server_msg &msg) {
    if (ready == 0 && (msg.turn_direction == 1 || msg.turn_direction == 2))
        std::cout << name << " is ready!" << std::endl;
    ready = ready || msg.turn_direction == 1 || msg.turn_direction == 2;
    last_turn = msg.turn_direction;
}

int32_t Player::is_ready() {
    return ready;
}

const std::string Player::get_name() {
    return name;
}

void Player::set_worm(const std::shared_ptr <Worm> &ptr) {
    worm = ptr;
}

int32_t Player::get_last_turn() {
    return last_turn;
}

void Player::set_ready(const bool rdy) {
    ready = rdy;
}
