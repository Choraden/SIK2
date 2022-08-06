#ifndef SIK2_WORM_H
#define SIK2_WORM_H

#define PI 3.14159265
#define STRAIGHT 0
#define RIGHT 1
#define LEFT 2
#define NEW_GAME 0
#define PIXEL 1
#define PLAYER_ELIMINATED 2
#define GAME_OVER 3
#define CONTINUE 0

#include "board.h"

#include <cstdint>
#include <cmath>
#include <string>

class Worm {
public:
    Worm(const std::string &n, const int32_t ts) : name(n), turning_speed(ts) {}

    void set_position(const double new_x, const double new_y);

    void set_turn(const int32_t turn);

    /** Change worm position, return the right response */
    int32_t move(Board &board);

    void set_id(int32_t new_id);

    int32_t get_id();

    uint32_t get_x();

    uint32_t get_y();

    void set_direction(int32_t dir);

    bool operator<(const Worm &w) const;

    const std::string &get_name();


private:
    std::string name;
    int32_t id;
    double x;
    double y;
    int32_t direction;
    int32_t turning_speed;
    int32_t last_turn;

    double deg_to_rad(const double degree);

    /** Change worm direction based on its last turn */
    void change_direction();

};


#endif //SIK2_WORM_H
