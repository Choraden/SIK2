#include "server.h"

void Server::set_parameters(const int argc, char *const argv[]) {
    for (;;) {
        switch (getopt(argc, argv, "p:s:t:v:w:h:")) {
            case 'p':
                port = validate_parameter(optarg, 1, 65535);
                break;

            case 's':
                seed = validate_parameter(optarg, 1, 4294967295);
                break;

            case 't':
                turning_speed = validate_parameter(optarg, 1, 90);
                break;

            case 'v':
                rounds_per_sec = validate_parameter(optarg, 1, 250);
                break;

            case 'w':
                width = validate_parameter(optarg, 10, 4000);
                board.set_width_height(width, height);
                break;

            case 'h':
                height = validate_parameter(optarg, 10, 4000);
                board.set_width_height(width, height);
                break;

            case -1:
                /** There are arguments which are not familiar to server */
                if (argc > optind) {
                    throw std::string("invalid parameter");
                }
                return;

            default:
                throw std::string("invalid parameter");

        }
    }
}

int64_t Server::validate_parameter(const std::string &param, const int64_t lo, const int64_t hi) {
    if (param.size() == 1 && !isdigit(param[0])) {
        throw std::string("invalid parameter");
    }
    int64_t res = 0;
    int64_t sign = 1;
    for (size_t i = 0; i < param.size(); i++) {
        if (isdigit(param[i])) {
            res *= 10;
            res += param[i] - '0';
        } else if (i == 0 && param[i] == '-') {
            sign = -1;
        } else {
            throw std::string("invalid parameter");
        }
    }
    res *= sign;
    if (res >= lo && res <= hi) {
        return res;
    }

    throw std::string("parameter out of range");
}

void Server::test_parameters() noexcept {
    std::cout << "port " << port << std::endl;
    std::cout << "seed " << seed << std::endl;
    std::cout << "ts " << turning_speed << std::endl;
    std::cout << "rps " << rounds_per_sec << std::endl;
    std::cout << "w " << width << std::endl;
    std::cout << "h " << height << std::endl;
}

void Server::listen() {
    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock < 0)
        throw std::string("socket failure");

    /** Listening on ipv6 and ipv4. */
    int ipv6only = 0;
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6only, sizeof(ipv6only)) != 0)
        throw std::string("set_sock_opt ipv6 + ipv4 failure");


    /** Make socket nonblocking. */
    struct timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 10;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0)
        throw std::string("set socket nonblocking failure");

    struct sockaddr_in6 server_address{};
    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_port = htons(port);

    /** Bind the socket to a concrete address. */
    if (bind(sock, (struct sockaddr *) &server_address,
             (socklen_t)
        sizeof(server_address)) < 0)
    throw std::string("binding socket failure");

    std::cout << "server listening on port " << port << std::endl;
}

uint32_t Server::rand() noexcept {
    uint32_t res = seed;
    seed = (seed * 279410273) % 4294967291;
    return res;
}

void Server::read_datagram() {
    struct sockaddr_in6 client_addr{};
    socklen_t addr_len;
    client_server_msg msg;
    memset(&msg, 0, sizeof(msg));
    int32_t msg_len = recvfrom(sock, &msg, sizeof(msg), 0, (struct sockaddr *) &client_addr,
                               &addr_len);
    msg.session_id = be64toh(msg.session_id);
    msg.next_expected_event_no = be32toh(msg.next_expected_event_no);
    if (msg_len >= CLIENT_SERVER_MSG_MIN_LEN
        && msg_len <= CLIENT_SERVER_MSG_LEN
        && validate_player_name(msg, msg_len - CLIENT_SERVER_MSG_MIN_LEN)) {
        handle_msg(msg, client_addr);
    }
}

void Server::handle_msg(const client_server_msg &msg, const struct sockaddr_in6 &addr) {
    std::string id = get_client_id(addr);
    try {
        const auto &user = users.at(id);
        if (user->get_session_id() == msg.session_id) {
            user->set_timer(user_timer_spec);
            if (in_game) {
                user->move(msg);
            } else {
                user->set_ready(msg);
            }
            response(msg.next_expected_event_no, addr);

        } else if (user->get_session_id() < msg.session_id) {
            disconnect_user(id);
            add_new_user(msg, id, addr);
        }
    } catch (std::out_of_range &e) { /// No client of such id.
        if (users.size() < MAX_USERS) {
            add_new_user(msg, id, addr);
        }
    }
}

void Server::run() {
    set_user_timer_spec();
    set_game_timer_spec();
    listen();
    while (true) {
        if (in_game) {
            turn_timeout();
        } else {
            prepare_game();
        }
        read_datagram();
        check_players_timeout();
    }
}

void Server::set_user_timer_spec() noexcept {
    user_timer_spec.it_value.tv_sec = 2;
    user_timer_spec.it_value.tv_nsec = 0;
    user_timer_spec.it_interval.tv_sec = 0;
    user_timer_spec.it_interval.tv_nsec = 0;
}

void Server::set_game_timer_spec() noexcept {
    game_timer_spec.it_value.tv_sec = 0;
    game_timer_spec.it_value.tv_nsec = 1000000000 / rounds_per_sec;
    game_timer_spec.it_interval.tv_sec = 0;
    game_timer_spec.it_interval.tv_nsec = 1000000000 / rounds_per_sec;
}

bool Server::validate_player_name(const client_server_msg &msg, const int32_t len) noexcept {
    for (int32_t i = 0; i < len; i++) {
        if (msg.player_name[i] < 33 || msg.player_name[i] > 126) {
            return false;
        }
    }
    return true;
}

const std::string Server::get_client_id(const struct sockaddr_in6 &addr) {
    char ipv6[INET6_ADDRSTRLEN];
    memset(ipv6, 0, sizeof(ipv6));
    inet_ntop(AF_INET6, addr.sin6_addr.s6_addr, ipv6, INET6_ADDRSTRLEN);
    return std::string(ipv6) + "$" + std::to_string(addr.sin6_port);
}

void Server::add_new_user(const client_server_msg &msg, const std::string &id,
                          const struct sockaddr_in6 &addr) {
    const auto &name = get_username(msg);
    if (name) {
        for (const auto &u: users)
            if (u.second->get_name() == name)
                return;

        Player p(msg.session_id, addr, name.value(), msg.turn_direction);
        p.set_timer(user_timer_spec);
        users.emplace(id, std::make_shared<Player>(p));
    } else {
        Viewer v(msg.session_id, addr);
        v.set_timer(user_timer_spec);
        users.emplace(id, std::make_shared<Viewer>(v));
    }
    response(msg.next_expected_event_no, addr);
}

const std::optional <std::string> Server::get_username(const client_server_msg &msg) {
    std::string s(msg.player_name);
    if (s.empty()) {
        return std::nullopt;
    }
    return s;
}

void Server::disconnect_user(const std::string &id) {
    try {
        auto &user = users.at(id);
        user->close_fd();
        users.erase(id);
        std::string usr_name = user->get_name();
        if (usr_name != " ")
            std::cout << "Disconnecting player " << usr_name << std::endl;
        else
            std::cout << "Disconnecting viewer" << std::endl;
    } catch (...) {
        std::cerr << "Exception in disconnecting user!" << std::endl;
    }
}

void Server::check_players_timeout() {
    std::vector <std::string> to_delete;
    for (const auto &u : users) {
        if (u.second->is_expired()) {
            to_delete.push_back(u.first);
        }
    }

    for (const auto &id: to_delete) {
        disconnect_user(id);
    }
}

void Server::make_turn() {
    for (size_t i = 0; i < worms.size(); i++) {
        int32_t rv = worms[i]->move(board);
        if (rv == PIXEL) {
            pixel_event(worms[i]->get_id(), worms[i]->get_x(), worms[i]->get_y());
        } else if (rv == PLAYER_ELIMINATED) {
            player_eliminated_event(worms[i]->get_id());
            worms.erase(worms.begin() + i);
            i--;
            if (worms.size() < 2) {
                game_over();
                return;
            }
        }
    }
}

void Server::prepare_game() {
    int32_t ready = 0;
    for (auto &user: users) {
        int32_t rdy = user.second->is_ready();
        if (rdy > 0) {
            ready++;
        } else if (!rdy) { /// If one player isn't ready the game won't begin.
            return;
        }
    }
    if (ready >= 2) { /// At least 2 players.
        create_worms();
        start_game();
    }
}

void Server::start_game() {
    events.clear();
    board.new_();
    game_id = rand();
    new_game_event();
    set_round_time();
    for (size_t i = 0; i < worms.size(); i++) {
        double x = rand() % width + 0.5;
        double y = rand() % height + 0.5;
        worms[i]->set_direction(rand() % 360);
        if (board.get_box(x, y)) {
            board.take_box(x, y);
            worms[i]->set_position(x, y);
            pixel_event(i,
                        static_cast<uint32_t>(std::floor(x)),
                        static_cast<uint32_t>(std::floor(y)));
        } else {
            player_eliminated_event(i);
            worms.erase(worms.begin() + i);
            i--; /// Move iterator one step back, since we erased one worm.
        }
    }
    in_game = true;
}

void Server::create_worms() {
    for (auto &user: users) {
        if (user.second->is_ready() == READY) {
            std::shared_ptr <Worm> ptr = std::make_shared<Worm>(Worm(user.second->get_name(), turning_speed));
            worms.push_back(ptr);
            user.second->set_worm(ptr);
            ptr->set_turn(user.second->get_last_turn());
        }
    }
    std::sort(worms.begin(), worms.end(),
              [](const std::shared_ptr <Worm> &a, const std::shared_ptr <Worm> &b) { return *a < *b; });
    for (size_t i = 0; i < worms.size(); i++) {
        worms[i]->set_id(i);
    }
}

void Server::set_round_time() {
    timer.revents = 0;
    timerfd_settime(timer.fd, 0, &game_timer_spec, NULL);
}

void Server::turn_timeout() {
    int32_t rv = poll(&timer, 1, 0);
    if (rv && timer.revents & POLLIN) {
        uint64_t buff;
        /** Read from timer's file descriptor to clear it and allow
         * interval usage. */
        if (read(timer.fd, &buff, sizeof(buff)) < 0)
            std::cerr << "timer error" << std::endl;
        timer.revents = 0;

        make_turn();
    }
}

void Server::new_game_event() {
    Event e;
    std::cout << "NEW GAME" << std::endl;
    e.add_dword(events.size());
    e.add_byte(NEW_GAME);
    e.add_dword(width);
    e.add_dword(height);
    for (const auto &w: worms) {
        e.add_name(w->get_name());
    }
    e.checksum();
    events.push_back(e);
    broadcast_new_event();
}

void Server::pixel_event(uint8_t num, uint32_t x, uint32_t y) {
    Event e;
    e.add_dword(events.size());
    e.add_byte(PIXEL);
    e.add_byte(num);
    e.add_dword(x);
    e.add_dword(y);
    e.checksum();
    events.push_back(e);
    broadcast_new_event();
}

void Server::player_eliminated_event(uint8_t num) {
    Event e;
    e.add_dword(events.size());
    e.add_byte(PLAYER_ELIMINATED);
    e.add_byte(num);
    e.checksum();
    events.push_back(e);
    broadcast_new_event();
}

void Server::game_over_event() {
    Event e;
    std::cout << "GAME OVER" << std::endl;
    e.add_dword(events.size());
    e.add_byte(GAME_OVER);
    e.checksum();
    events.push_back(e);
    broadcast_new_event();
}

void Server::broadcast_new_event() {
    uint8_t buff[MAX_UDP_MSG_LEN];
    uint32_t be_game_id = htobe32(game_id);
    memcpy(buff, &be_game_id, DWORD);
    const auto &event = events.back().get_content();
    memcpy(buff + DWORD, &event[0], event.size());
    for (const auto &user: users) {
        sendto(sock, buff, event.size() + DWORD, 0, (struct sockaddr *) &user.second->get_addr(),
               sizeof(user.second->get_addr()));
    }
}

void Server::response(const uint32_t next_expected_event_no, const struct sockaddr_in6 &addr) {
    uint8_t buff[MAX_UDP_MSG_LEN];
    uint32_t be_game_id = htobe32(game_id);
    memcpy(buff, &be_game_id, DWORD);
    uint32_t msg_len = DWORD;
    for (size_t i = next_expected_event_no; i < events.size(); i++) {
        const auto &event = events[i].get_content();
        if (msg_len + event.size() > MAX_UDP_MSG_LEN) {
            sendto(sock, buff, msg_len, 0, (struct sockaddr *) &addr, sizeof(addr));
            msg_len = DWORD;
        }
        memcpy(buff + msg_len, &event[0], event.size());
        msg_len += event.size();
    }
    if (msg_len > DWORD) {
        sendto(sock, buff, msg_len, 0, (struct sockaddr *) &addr, sizeof(addr));
    }

}

void Server::game_over() {
    in_game = false;
    game_over_event();
    worms.clear();
    for (auto const &u : users) {
        u.second->set_ready(false);
    }
}






