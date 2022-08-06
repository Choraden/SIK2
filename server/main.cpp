#include "server.h"

int main(int argc, char *argv[]) {
    try {
        Server server;
        server.set_parameters(argc, argv);
        server.test_parameters();
        server.run();
    } catch (std::string &s) {
        std::cerr << "Exception! Msg: " << s << std::endl;
        exit(1);
    }
}