#pragma once

#include <stdint.h>
#include <vector>
#include "SerialPort.hpp"

class DataLinkLayer {
    public:
        DataLinkLayer(SerialPort& port);
        void sendData(const std::vector<uint8_t>& data);
        void receiveLoop();

    private:
        void sendFrame(uint8_t type, const std::vector<uint8_t>& payload);
        void handleFrame(uint8_t type, const std::vector<uint8_t>& payload);

        SerialPort& serial;
};