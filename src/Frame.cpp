#include "Frame.hpp"

std::vector<uint8_t> Frame::encode(uint8_t type, const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> frame {};
    frame.push_back(0x7E);

    frame.push_back(type);
    frame.push_back(payload.size());
    frame.insert(frame.end(), payload.begin(), payload.end());

    frame.push_back(0x7E);
    return frame;
}

bool Frame::decode(const std::vector<uint8_t>& raw, uint8_t& type, std::vector<uint8_t>& payload) {
    if(raw.size() < 4) return false;
    if((raw.front() != 0x7E) || (raw.back() != 0x7E)) return false;

    type = raw[1];
    uint8_t length = raw[2];

    if (length != raw.size() - 4) return false;

    payload.assign(raw.begin() + 3, raw.begin() + 3 + length);
    return true;
}

