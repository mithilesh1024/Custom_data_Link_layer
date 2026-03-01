#include "ByteStuffer.hpp"

const uint8_t ByteStuffer::DELIM;
const uint8_t ByteStuffer::ESC;
const uint8_t ByteStuffer::MASK;

std::vector<uint8_t> ByteStuffer::stuff(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> result;
    for (uint8_t b : data) {
        if (b == ByteStuffer::DELIM || b == ByteStuffer::ESC) {
            result.push_back(ByteStuffer::ESC);
            result.push_back(b ^ ByteStuffer::MASK);
        } else {
            result.push_back(b);
        }
    }
    return result;
}

std::vector<uint8_t> ByteStuffer::unstuff(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> result;
    for (size_t i = 0; i < data.size(); ++i) {
        if (data[i] == ByteStuffer::ESC) {
            i++; // Skip the escape byte
            if (i < data.size()) {
                result.push_back(data[i] ^ ByteStuffer::MASK);
            }
        } else {
            result.push_back(data[i]);
        }
    }
    return result;
}