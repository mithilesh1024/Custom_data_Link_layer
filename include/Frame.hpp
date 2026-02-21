#pragma once

/*
FRAME STRUCTURE

| 0x7E | TYPE | LENGTH | PAYLOAD | 0x7E |
*/

#include <stdint.h>
#include <vector>

class Frame {
    public:
        static std::vector<uint8_t> encode(uint8_t type, const std::vector<uint8_t>& payload);
        static bool decode(const std::vector<uint8_t>& raw, uint8_t& type, std::vector<uint8_t>& payload);
};