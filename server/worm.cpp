#include "worm.h"

void Worm::set_position(const double new_x, const double new_y) {
    x = new_x;
    y = new_y;
}

void Worm::change_direction() {
    if (last_turn == RIGHT) {
        direction = (direction + turning_speed) % 360;
    } else if (last_turn == LEFT) {
        direction = (direction - turning_speed) % 360;
    }
}

void Worm::set_turn(const int32_t turn) {
    last_turn = turn;
}

double Worm::deg_to_rad(double degree) {
    return (degree * (PI / 180.0));
}

int32_t Worm::move(Board &board) {
    int32_t prev_x = std::floor(x);
    int32_t prev_y = std::floor(y);
    change_direction();
    x += cos(deg_to_rad(direction));
    y += sin(deg_to_rad(direction));
    int32_t new_x = std::floor(x);
    int32_t new_y = std::floor(y);
    if (prev_x == new_x && prev_y == new_y) {
        return CONTINUE;
    } else if (board.get_box(new_x, new_y)) {
        board.take_box(new_x, new_y);
        return PIXEL;
    } else {
        return PLAYER_ELIMINATED;
    }
}

bool Worm::operator<(const Worm &w) const {
    return name.compare(w.name) < 0;
}

void Worm::set_direction(int32_t dir) {
    direction = dir;
}

const std::string &Worm::get_name() {
    return name;
}

void Worm::set_id(int32_t new_id) {
    id = new_id;
}

int32_t Worm::get_id() {
    return id;
}

uint32_t Worm::get_x() {
    return std::floor(x);
}

uint32_t Worm::get_y() {
    return std::floor(y);
}
