#pragma once

#include <vector>
#include <stdint.h>

struct BufferWriter {
    std::vector<uint8_t>& buffer;

    void writeUint8(uint8_t value) {
        buffer.push_back(value);
    }

    void writeUint16(uint16_t value) {
        writeUint8(static_cast<uint8_t>(value >> 8));
        writeUint8(static_cast<uint8_t>(value & 0xFF));
    }

    void writeBytes(const std::vector<uint8_t>& data) {
        buffer.insert(buffer.end(), data.begin(), data.end());
    }
};