#pragma once

#include <stdint.h>
#include <vector>
#include <thread>
#include "SerialPort.hpp"
#include "Frame.hpp"

typedef void (*CallbackFunc)(void*, void*);

class DataLinkLayer {
    public:
        DataLinkLayer(SerialPort& port, CallbackFunc cb);
        ~DataLinkLayer();
        void sendData(const std::vector<uint8_t>& data);
        void convertDataToPayload(const std::string str, std::vector<uint8_t>& payload);
        
    private:
        void start();
        void receiveLoop();
        void sendFrame(uint8_t type, const std::vector<uint8_t>& payload);
        void handleFrame(uint8_t type, const std::vector<uint8_t>& payload);

        SerialPort& serial;
        std::thread listening_thread;
        CallbackFunc callback;
};