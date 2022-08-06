#include "board.h"

void Board::take_box(int32_t x, int32_t y) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        int32_t b = x * height + y;
        if (b < width * height) {
            box[b] = false;
        }
    }
}

void Board::take_box(double x, double y) {
    int32_t int_x = std::floor(x);
    int32_t int_y = std::floor(y);
    return take_box(int_x, int_y);
}

void Board::new_() {
    box = std::vector<bool>(width * height, true);
}

void Board::clean_all() {
    box.clear();
}

bool Board::get_box(int32_t x, int32_t y) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        int32_t b = x * height + y;
        if (b < width * height) {
            return box[b];
        }
    }
    return false;
}

bool Board::get_box(double x, double y) {
    int32_t int_x = std::floor(x);
    int32_t int_y = std::floor(y);
    return get_box(int_x, int_y);
}

void Board::set_width_height(int32_t w, int32_t h) {
    width = w;
    height = h;
}
