#include "DataLinkLayer.hpp"

DataLinkLayer::DataLinkLayer(SerialPort& port, CallbackFunc cb) : serial(port), callback(cb) {
    start();
}

DataLinkLayer::~DataLinkLayer(){
    if(listening_thread.joinable()){
        listening_thread.join();
    }
}

void DataLinkLayer::start() {
    if(listening_thread.joinable()){
        listening_thread.join();
    }
    listening_thread = std::thread(&DataLinkLayer::receiveLoop, this);
}

void DataLinkLayer::sendData(const std::vector<uint8_t>& data, Frame::message_type ack) {

    std::queue<std::vector<uint8_t>> packets = Frame::breakMessage(data);
    int current_seq = 0;
    while(!packets.empty()){
        if(packets.size() == 1) {
            ack = Frame::LAST_DATA;
        }
        std::vector<uint8_t> curr_msg = packets.front();
        #ifdef COMM_TEST_MODE
            std::cout << "chunks: ";
            std::cout.write(reinterpret_cast<char*>(curr_msg.data()), curr_msg.size());
            std::cout << std::endl;
        #endif
        auto frame = Frame::encode(0x01, curr_msg, current_seq, ack);

        {
            // LOCK BEFORE SENDING
            std::unique_lock<std::mutex> lock(ack_mutex);
            ack_received = false; // Clear previous flag

            // Send the bytes while we "intend" to listen
            serial.writeBytes(frame.data(), frame.size());

            // 2. Wait for ACK
            // By being already locked, handleFrame cannot update ack_received 
            // until we enter wait_for (which releases the lock)
            bool signaled = ack_cv.wait_for(lock, std::chrono::milliseconds(2000), [this, current_seq] { 
                return ack_received && last_ack_seq == current_seq; 
            });

            if (signaled) {
                packets.pop();
                current_seq += 1 ; 
            } else {
                // Timeout logic: just loops back and tries again
                std::cout << "Timeout/Missed ACK for seq " << (int)current_seq << ". Retrying..." << std::endl;
            }
        }
    }
}

void DataLinkLayer::sendAck(const uint8_t seqNum, Frame::message_type ack) {
    auto frame = Frame::encode(0x01, {}, seqNum, ack);
    serial.writeBytes(frame.data(), frame.size());
}

void DataLinkLayer::receiveLoop() {
    std::vector<uint8_t> buffer(256);

    while (true) {
        #ifdef COMM_TEST_MODE
            std::cout<<"Waiting for Data\n";
        #endif
        ssize_t n = serial.readBytes(buffer.data(), buffer.size());
        #ifdef COMM_TEST_MODE
            std::cout<<"Got "<<n<<" bytes of data\n";
        #endif
        if (n > 0) {
            std::vector<uint8_t> raw(buffer.begin(), buffer.begin() + n);
            enum Frame::MessageQueueStatus already_received;
            int seq_num;
            auto chunk_msg = Frame::updateVector(raw, already_received, seq_num);
            handleFrame(chunk_msg);
        }
    }
}

void DataLinkLayer::convertDataToPayload(const std::string str, std::vector<uint8_t>& payload){
    for(int i=0;i<str.length();i++){
        payload.push_back(static_cast<uint8_t>(str[i]));
    }
}

void DataLinkLayer::sendFrame(uint8_t type, const std::vector<uint8_t>& payload) {

}

void DataLinkLayer::handleFrame(Frame::SerialData &data) {
    switch(data.header.type) {
        case Frame::message_type::DATA:
            sendAck(data.header.seqNum);
            break;
        case Frame::message_type::LAST_DATA:
        {
            #ifdef COMM_TEST_MODE
                std::cout<<"Sending ack for last message\n";
            #endif
            sendAck(data.header.seqNum);
            std::vector<uint8_t> data = Frame::decode();
            #ifdef COMM_TEST_MODE
                std::cout<<"size of data "<<data.size()<<"\n";
            #endif
            // for(auto msg : data) {
                #ifdef COMM_TEST_MODE
                    std::cout << "Received: ";
                    std::cout.write(reinterpret_cast<char*>(msg.payload.data()), msg.payload.size());
                    std::cout << std::endl;
                #endif
                
                callback(data, serial.getDeviceName().data());
            // }
            break;
        }
        case Frame::message_type::ACK: 
        {
            std::lock_guard<std::mutex> lock(ack_mutex);
            last_ack_seq = data.header.seqNum;
            ack_received = true;
            ack_cv.notify_all(); // Wake up sendData
            break;
        }
        default:
            std::cout<<"Ack for "<<(int)data.header.seqNum<<std::endl;
            break;
    }
}
