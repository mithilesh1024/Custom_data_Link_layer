#pragma once

#include <stdint.h>
#include <vector>

class ByteStuffer {
    private:
        static const uint8_t DELIM = 0x7D;
        static const uint8_t ESC   = 0x7C;
        static const uint8_t MASK  = 0x20;

        // Takes clean data and returns the "disguised" version
        static std::vector<uint8_t> stuff(const std::vector<uint8_t>& data);

        // Takes stuffed data and returns the original "clean" version
        static std::vector<uint8_t> unstuff(const std::vector<uint8_t>& data);

        friend class Frame;
};