#pragma once

/*
FRAME STRUCTURE

| 0x7D | TYPE | LENGTH | PAYLOAD | CRC_H | CRC_L | 0x7D |
*/

#include <stdint.h>
#include <iostream>
#include <vector>
#include <iomanip>

#define PACKET_DELIMITOR 0x7D

class Frame {
    public:
        struct SerialData {
            uint8_t type;
            uint8_t length;
            std::vector<uint8_t> payload;
        };

        static std::vector<uint8_t> encode(uint8_t type, const std::vector<uint8_t>& payload);
        static std::vector<Frame::SerialData> decode(const std::vector<uint8_t>& raw);
    
    private:
        static uint16_t crc16_ccitt(const uint8_t *data, int length);
        static void printHex(const std::vector<uint8_t> raw);
    //     SerialData extractPacket(const)

};