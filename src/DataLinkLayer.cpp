#include "DataLinkLayer.hpp"

DataLinkLayer::DataLinkLayer(SerialPort& port) : serial(port) {

}

void DataLinkLayer::sendData(const std::vector<uint8_t>& data) {
    auto frame = Frame::encode(0x01, data);
    serial.writeBytes(frame.data(), frame.size());
}

void DataLinkLayer::receiveLoop() {
    std::vector<uint8_t> buffer(256);

    while (true) {
        ssize_t n = serial.readBytes(buffer.data(), buffer.size());
        if (n > 0) {
            std::vector<uint8_t> raw(buffer.begin(), buffer.begin() + n);

            uint8_t type;
            std::vector<uint8_t> payload;

            if (Frame::decode(raw, type, payload)) {
                std::cout << "Received: ";
                std::cout.write(reinterpret_cast<char*>(payload.data()), payload.size());
                std::cout << std::endl;
            }
        }
    }
}

void DataLinkLayer::sendFrame(uint8_t type, const std::vector<uint8_t>& payload) {

}

void DataLinkLayer::handleFrame(uint8_t type, const std::vector<uint8_t>& payload) {

}
