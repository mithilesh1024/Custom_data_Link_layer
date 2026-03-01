#pragma once

#include <vector>
#include <stdint.h>

struct BufferReader {
    const std::vector<uint8_t>& buffer;
    size_t pos = 0;

    uint8_t readUint8() {
        if (pos >= buffer.size()) return 0;
        return buffer[pos++];
    }

    uint16_t readUint16() {
        uint8_t high = readUint8();
        uint8_t low = readUint8();
        return (static_cast<uint16_t>(high) << 8) | low;
    }

    std::vector<uint8_t> readBytes(size_t size) {
        if (pos + size > buffer.size()) {
            size = buffer.size() - pos;
        }
        std::vector<uint8_t> result(buffer.begin() + pos, buffer.begin() + pos + size);
        pos += size;
        return result;
    }
};