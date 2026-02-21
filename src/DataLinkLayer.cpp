#include "DataLinkLayer.hpp"

DataLinkLayer::DataLinkLayer(SerialPort& port) : serial(port) {

}

void DataLinkLayer::sendData(const std::vector<uint8_t>& data) {

}

void DataLinkLayer::receiveLoop() {

}

void DataLinkLayer::sendFrame(uint8_t type, const std::vector<uint8_t>& payload) {

}

void DataLinkLayer::handleFrame(uint8_t type, const std::vector<uint8_t>& payload) {

}
