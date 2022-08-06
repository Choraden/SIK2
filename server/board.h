#ifndef SIK2_BOARD_H
#define SIK2_BOARD_H

#include <vector>
#include <cstdint>
#include <cmath>

class Board {
public:
    Board(int32_t w, int32_t h) : width(w), height(h), box() {}

    void new_();

    bool get_box(int32_t x, int32_t y);

    bool get_box(double x, double y);

    void take_box(int32_t x, int32_t y);

    void take_box(double x, double y);

    void set_width_height(int32_t w, int32_t h);

    void clean_all();

private:
    int32_t width;
    int32_t height;
    std::vector<bool> box;
};

#endif //SIK2_BOARD_H
