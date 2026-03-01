#include "Frame.hpp"

std::vector<std::optional<SerialData>> Frame::queue;

std::vector<uint8_t> Frame::encode(uint8_t type, const std::vector<uint8_t>& payload, const uint8_t seqNum, const enum MessageType ack ) {
    std::vector<uint8_t> frame {};
    BufferWriter buffer{frame};
    
    // 1. Start with the Delimiter
    buffer.writeUint8(ByteStuffer::DELIM);

    // 2. Prepare the Header & Payload
    std::vector<uint8_t> contents;
    BufferWriter stuffing_buf{contents};
    stuffing_buf.writeUint8(static_cast<uint8_t>(ack));
    stuffing_buf.writeUint8(seqNum);
    stuffing_buf.writeUint8(type);
    stuffing_buf.writeUint8(payload.size()); // Original size
    stuffing_buf.writeBytes(payload);

    // 3. Calculate CRC on the CLEAN data
    uint16_t crc = crc16_ccitt(payload.data(), payload.size());
    stuffing_buf.writeUint16(crc); 

    // 4. Stuff the contents (Disguise any 0x7D or 0x7C)
    std::vector<uint8_t> stuffedBody = ByteStuffer::stuff(contents);
    buffer.writeBytes(stuffedBody);

    // 5. End with the Delimiter
    buffer.writeUint8(ByteStuffer::DELIM);

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

SerialData Frame::updateVector(const std::vector<uint8_t>& payload, enum MessageQueueStatus &alreadyRecieved, int &seq_num) {
    std::vector<std::vector<uint8_t>> packets = getPackets(payload);
    std::vector<SerialData> res = convertEachMessageToStructure(packets);
    
    #ifdef COMM_TEST_MODE
        printFrame(res);
    #endif

    for (auto r : res) {
        if(r.header.type == MessageType::ACK) {
            return r; 
        }
        int seq = r.header.seqNum;
        seq_num = seq;
        if(seq >= Frame::queue.size()){
            Frame::queue.resize(seq + 1);
            std::optional<SerialData> temp = r;
            Frame::queue[seq] = r;
            alreadyRecieved = MessageQueueStatus::NEW_ADDED;
        }else {
            if(!Frame::queue[seq].has_value()){
                alreadyRecieved = MessageQueueStatus::NEW_ADDED;
                std::optional<SerialData> temp = r;
                Frame::queue.insert(Frame::queue.begin() + seq, temp);
            }else {
                alreadyRecieved = MessageQueueStatus::ALREADY_PRESENT;
            }
        }
        return r;
    }

    return {};
}

void Frame::printFrame(std::vector<SerialData> frame) {
    for(auto data : frame) {
        std::string typeStr;
        switch (data.header.type) {
            case MessageType::ACK:       typeStr = "ACK"; break;
            case MessageType::DATA:      typeStr = "DATA"; break;
            case MessageType::LAST_DATA: typeStr = "LAST_DATA"; break;
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

bool Frame::isLastMessage(const SerialData& msg) {
    if(msg.header.type == MessageType::LAST_DATA) {
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

    std::vector<uint8_t> currentPacket;
    bool inPacket = false;
    bool escaped = false;

    for (size_t i = 0; i < raw.size(); i++) {
        uint8_t byte = raw[i];

        if (!inPacket) {
            if (byte == ByteStuffer::DELIM) {
                inPacket = true;
                currentPacket.push_back(byte);
            }
        } else {
            currentPacket.push_back(byte);

            if (escaped) {
                // This byte was preceded by 0x7C, so it's just data.
                // We don't check if it's a delimiter here.
                escaped = false; 
            } else if (byte == 0x7C) {
                // We hit an escape byte! 
                // The NEXT byte must be treated as data.
                escaped = true;
            } else if (byte == ByteStuffer::DELIM) {
                // A delimiter with no escape before it? 
                // That's the real end of the packet.
                packets.push_back(currentPacket);
                currentPacket.clear();
                inPacket = false;
            }
        }
    }
    #ifdef COMM_TEST_MODE
        std::cout<<"number of packets "<<packets.size()<<std::endl;
    #endif
    return packets;
}

std::vector<SerialData> Frame::convertEachMessageToStructure(std::vector<std::vector<uint8_t>>& packets) {
    std::vector<SerialData> res;
    for(auto p : packets){
        std::vector<uint8_t> clean = ByteStuffer::unstuff(p);
        #ifdef COMM_TEST_MODE
            printHex(clean);
        #endif
        BufferReader reader {clean};
        
        SerialData sd;
        int index = 1;
        reader.readUint8();
        sd.header.type = static_cast<MessageType>(reader.readUint8());
        sd.header.seqNum = reader.readUint8();

        sd.type = reader.readUint8();
        sd.length = reader.readUint8();
    
        sd.payload = reader.readBytes(sd.length);

        uint16_t received_crc = reader.readUint16();
        uint16_t cal_crc = crc16_ccitt(sd.payload.data(), sd.length);  
        if(received_crc == cal_crc)
            res.push_back(sd);
        else{
            std::cout<<"Error in recieved crc\n";
        }
    }

    return res;
}


