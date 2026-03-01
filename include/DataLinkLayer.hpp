#pragma once

#include <stdint.h>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "SerialPort.hpp"
#include "Frame.hpp"

typedef void (*CallbackFunc)(std::vector<uint8_t>, void*);

class DataLinkLayer {
    public:
        DataLinkLayer(SerialPort& port, CallbackFunc cb);
        ~DataLinkLayer();
        void sendData(const std::vector<uint8_t>& data, Frame::message_type ack = Frame::message_type::DATA);
        void convertDataToPayload(const std::string str, std::vector<uint8_t>& payload);
        
    private:
        void start();
        void receiveLoop();
        void sendFrame(uint8_t type, const std::vector<uint8_t>& payload);
        void handleFrame(Frame::SerialData &data);
        void sendAck(const uint8_t seqNum, Frame::message_type ack = Frame::message_type::ACK);

        SerialPort& serial;
        std::thread listening_thread;
        CallbackFunc callback;

        std::mutex ack_mutex;
        std::condition_variable ack_cv;
        int last_ack_seq = -1; 
        bool ack_received = false;
};