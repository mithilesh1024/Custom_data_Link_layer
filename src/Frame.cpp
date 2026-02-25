#include "Frame.hpp"

std::vector<uint8_t> Frame::encode(uint8_t type, const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> frame {};
    frame.push_back(PACKET_DELIMITOR);

    frame.push_back(type);
    frame.push_back(payload.size());
    frame.insert(frame.end(), payload.begin(), payload.end());

    uint16_t crc = crc16_ccitt(payload.data(), payload.size());
    frame.push_back((crc >> 8) & 0xFF);
    frame.push_back(crc & 0xFF);

    frame.push_back(PACKET_DELIMITOR);
    return frame;
}

std::vector<Frame::SerialData> Frame::decode(const std::vector<uint8_t>& raw) {
    std::vector<Frame::SerialData> res;
    std::vector<std::vector<uint8_t>> packets;
    for(int i=0;i<raw.size();i++){
        if(raw[i] == PACKET_DELIMITOR){
            std::vector<uint8_t> temp;
            temp.push_back(raw[i]);
            i++;
            while(raw[i] != PACKET_DELIMITOR){
                temp.push_back(raw[i]);
                i++;
            }
            if(raw[i] == PACKET_DELIMITOR) temp.push_back(raw[i]);
            packets.push_back(temp);
        }
    }
    #ifdef COMM_TEST_MODE
        std::cout<<"number of packets "<<packets.size()<<std::endl;
    #endif

    for(auto p : packets){
        #ifdef COMM_TEST_MODE
            printHex(p);
        #endif
        // std::cout.write(reinterpret_cast<char*>(p.data()), p.size());
        Frame::SerialData sd;
        if(p.size() < 4) continue;
        if((p.front() != PACKET_DELIMITOR) || (p.back() != PACKET_DELIMITOR)) continue;
    
        sd.type = p[1];
        sd.length = p[2];
    
        if (sd.length != p.size() - 6) continue;
    
        sd.payload.assign(p.begin() + 3, p.begin() + 3 + sd.length);

        uint16_t received_crc = (p[sd.length + 3] << 8) | p[sd.length + 4];
        uint16_t cal_crc = crc16_ccitt(sd.payload.data(), sd.length);  
        if(received_crc == cal_crc)
            res.push_back(sd);
        else{
            std::cout<<"Error in recieved crc\n";
        }
    }

    return res;
}

uint16_t Frame::crc16_ccitt(const uint8_t *data, int length) {
    uint16_t crc = 0xFFFF; // initial value
    for (int i = 0; i < length; i++) {
        crc ^= ((uint16_t)data[i] << 8);
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

void Frame::printHex(const std::vector<uint8_t> raw) {
    std::cout << std::hex << std::setfill('0'); // hex mode with zero-padding
    for (size_t i = 0; i < raw.size(); ++i) {
        std::cout << std::setw(2) << static_cast<int>(raw[i]);
        if (i != raw.size() - 1) std::cout << " ";
    }
    std::cout << std::dec << std::endl; // reset to decimal
}

