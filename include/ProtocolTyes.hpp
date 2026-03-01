#pragma once

#include <stdint.h>
#include <vector>

/**
 * @brief High-level message classification for the data link layer.
 */
enum class MessageType : uint8_t {
    ACK       = 0x00,
    DATA      = 0x01,
    LAST_DATA = 0x02
};

/**
 * @brief Status indicators for managing the receive queue.
 */
enum class MessageQueueStatus {
    ALREADY_PRESENT,
    NEW_ADDED
};

/**
 * @brief Internal header containing routing and sequencing info.
 */
struct SerialDataHeader {
    MessageType type;
    uint8_t seqNum;
};

/**
 * @brief The fully decoded data structure representing a single frame.
 */
struct SerialData {
    SerialDataHeader header;
    uint8_t type; // The 'TYPE' field from your frame structure
    uint8_t length;
    std::vector<uint8_t> payload;
};