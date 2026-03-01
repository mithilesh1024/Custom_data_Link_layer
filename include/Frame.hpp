#pragma once

/*
FRAME STRUCTURE

| 0x7D | TYPE | SEQ_NUM | TYPE | LENGTH | PAYLOAD | CRC_H | CRC_L | 0x7D |
*/

#include <stdint.h>
#include <iostream>
#include <vector>
#include <queue>
#include <iomanip>
#include <optional>
#include <algorithm>

#include "ByteStuffer.hpp"
#include "BufferReader.hpp"
#include "BufferWriter.hpp"
#include "ProtocolTyes.hpp"

#define PACKET_SIZE         0x02

class Frame {
    public:

        static std::vector<uint8_t> encode(uint8_t type, const std::vector<uint8_t>& payload, const uint8_t seqNum, const enum MessageType ack);
        static std::vector<uint8_t> decode();
        static std::queue<std::vector<uint8_t>> breakMessage(const std::vector<uint8_t>& payload);
        static void joinMessage(const std::vector<uint8_t>& chunks, std::vector<uint8_t>& final_payload);
        static SerialData updateVector(const std::vector<uint8_t>& payload, enum MessageQueueStatus &alreadyRecieved, int &seq_num);
        static bool isLastMessage(const SerialData& msg);
    
    private:
        static uint16_t crc16_ccitt(const uint8_t *data, int length);
        static void printHex(const std::vector<uint8_t> raw);
        static std::vector<std::vector<uint8_t>> getPackets(const std::vector<uint8_t> raw);
        static std::vector<SerialData> convertEachMessageToStructure(std::vector<std::vector<uint8_t>>& packets);
        static void printFrame(std::vector<SerialData>);

        static std::vector<std::optional<SerialData>> queue;

};