#ifndef SIK2_EVENT_H
#define SIK2_EVENT_H

#include <deque>
#include <cstdint>
#include <endian.h>
#include <string>
#include <algorithm>

#define BYTE 8
#define DWORD 4

/** Class stores events in binary format */
class Event {
public:
    Event() : content() {}

    Event(const Event &) = default;

    void add_byte(uint8_t byte);

    void add_dword(uint32_t dword);

    void add_name(const std::string &name);

    /** Counts crc checksum on content */
    void checksum();

    const std::deque <uint8_t> &get_content();


private:
    void add_length();

    std::deque <uint8_t> content;
};


#endif //SIK2_EVENT_H
