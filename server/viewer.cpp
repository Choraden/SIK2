#include "viewer.h"

void Viewer::move(const client_server_msg &) {
    return;
}

void Viewer::set_ready(const client_server_msg &) {
    return;
}

int32_t Viewer::is_ready() {
    return -1;
}

const std::string Viewer::get_name() {
    return " ";
}

void Viewer::set_worm(const std::shared_ptr <Worm> &) {
    return;
}

int32_t Viewer::get_last_turn() {
    return 0;
}

void Viewer::set_ready(const bool) {
    return;
}
