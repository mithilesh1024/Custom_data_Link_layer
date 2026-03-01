#include "Frame.hpp"

std::vector<std::optional<Frame::SerialData>> Frame::queue;

std::vector<uint8_t> Frame::encode(uint8_t type, const std::vector<uint8_t>& payload, const uint8_t seqNum, const enum message_type ack ) {
    std::vector<uint8_t> frame {};
    frame.push_back(PACKET_DELIMITOR);
    frame.push_back(ack);
    frame.push_back(seqNum);
    frame.push_back(type);
    frame.push_back(payload.size());
    frame.insert(frame.end(), payload.begin(), payload.end());

    uint16_t crc = crc16_ccitt(payload.data(), payload.size());
    frame.push_back((crc >> 8) & 0xFF);
    frame.push_back(crc & 0xFF);

    frame.push_back(PACKET_DELIMITOR);
    return frame;
}

std::queue<std::vector<uint8_t>> Frame::breakMessage(const std::vector<uint8_t>& payload) {
    std::queue<std::vector<uint8_t>> q;
    
    for(int i=0; i<payload.size(); i += PACKET_SIZE) {
        size_t size = static_cast<size_t>(std::min(PACKET_SIZE, static_cast<int>(payload.size() - i)));
        std::vector<uint8_t> chunks(payload.begin() + i, payload.begin() + i + size);
        q.push(chunks);
    }
    return q;
}

std::vector<uint8_t> Frame::decode() {
    std::vector<uint8_t> combined_msg;
    // std::vector<std::vector<uint8_t>> packets = getPackets(raw);
    for(auto msg : queue) {
        if(msg.has_value()){
            joinMessage(msg->payload, combined_msg);
        }
    }
    queue.clear();
    return combined_msg;
}

void Frame::joinMessage(const std::vector<uint8_t>& chunks, std::vector<uint8_t>& final_payload) {
    final_payload.insert(final_payload.end(), chunks.begin(), chunks.end());
}

Frame::SerialData Frame::updateVector(const std::vector<uint8_t>& payload, enum Frame::MessageQueueStatus &alreadyRecieved, int &seq_num) {
    std::vector<std::vector<uint8_t>> packets = getPackets(payload);
    std::vector<Frame::SerialData> res = convertEachMessageToStructure(packets);
    
    #ifdef COMM_TEST_MODE
        printFrame(res);
    #endif

    for (auto r : res) {
        if(r.header.type == Frame::message_type::ACK) {
            return r; 
        }
        int seq = r.header.seqNum;
        seq_num = seq;
        if(seq >= Frame::queue.size()){
            Frame::queue.resize(seq + 1);
            std::optional<Frame::SerialData> temp = r;
            Frame::queue[seq] = r;
            alreadyRecieved = Frame::MessageQueueStatus::NEW_ADDED;
        }else {
            if(!Frame::queue[seq].has_value()){
                alreadyRecieved = Frame::MessageQueueStatus::NEW_ADDED;
                std::optional<Frame::SerialData> temp = r;
                Frame::queue.insert(Frame::queue.begin() + seq, temp);
            }else {
                alreadyRecieved = Frame::MessageQueueStatus::ALREADY_PRESENT;
            }
        }
        return r;
    }

    return {};
}

void Frame::printFrame(std::vector<Frame::SerialData> frame) {
    for(auto data : frame) {
        std::string typeStr;
        switch (data.header.type) {
            case Frame::message_type::ACK:       typeStr = "ACK"; break;
            case Frame::message_type::DATA:      typeStr = "DATA"; break;
            case Frame::message_type::LAST_DATA: typeStr = "LAST_DATA"; break;
            default:                             typeStr = "UNKNOWN"; break;
        }
        
        std::cout << "--- Serial Data Frame ---" << "\n";
        std::cout << "Type (Header): " << typeStr << "\n";
        std::cout << "Seq Num:       " << (int)data.header.seqNum << "\n"; // Cast to int or it prints as a char
        std::cout << "Raw Type:      " << (int)data.type << "\n";
        std::cout << "Length:        " << (int)data.length << "\n";
        
        // 2. Print Payload in Hex
        std::cout << "Payload:       [ ";
        printHex(data.payload);
        std::cout << std::dec << "]" << "\n"; // Reset to decimal
        std::cout << "-------------------------" << std::endl;
    }
}

bool Frame::isLastMessage(const Frame::SerialData& msg) {
    if(msg.header.type == Frame::message_type::LAST_DATA) {
        return true;
    }
    return false;
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

std::vector<std::vector<uint8_t>> Frame::getPackets(const std::vector<uint8_t> raw) {
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
    return packets;
}

std::vector<Frame::SerialData> Frame::convertEachMessageToStructure(std::vector<std::vector<uint8_t>>& packets) {
    std::vector<Frame::SerialData> res;
    for(auto p : packets){
        #ifdef COMM_TEST_MODE
            printHex(p);
        #endif
        // std::cout.write(reinterpret_cast<char*>(p.data()), p.size());
        Frame::SerialData sd;
        int index = 1;
        if(p.size() < 8) continue;
        if((p.front() != PACKET_DELIMITOR) || (p.back() != PACKET_DELIMITOR)) continue;
        
        sd.header.type = static_cast<Frame::message_type>(p[index++]);
        sd.header.seqNum = p[index++];

        sd.type = p[index++];
        sd.length = p[index++];
    
        if (sd.length != p.size() - 8) continue;
    
        sd.payload.assign(p.begin() + 5, p.begin() + 5 + sd.length);

        uint16_t received_crc = (p[sd.length + 5] << 8) | p[sd.length + 6];
        uint16_t cal_crc = crc16_ccitt(sd.payload.data(), sd.length);  
        if(received_crc == cal_crc)
            res.push_back(sd);
        else{
            std::cout<<"Error in recieved crc\n";
        }
    }

    return res;
}


