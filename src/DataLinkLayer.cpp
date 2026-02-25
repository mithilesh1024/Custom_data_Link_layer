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

void DataLinkLayer::sendData(const std::vector<uint8_t>& data) {
    auto frame = Frame::encode(0x01, data);
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

            uint8_t type;
            std::vector<uint8_t> payload;
            std::vector<Frame::SerialData> data = Frame::decode(raw);
            #ifdef COMM_TEST_MODE
                std::cout<<"size of data "<<data.size()<<"\n";
            #endif
            for(auto msg : data) {
                #ifdef COMM_TEST_MODE
                    std::cout << "Received: ";
                    std::cout.write(reinterpret_cast<char*>(msg.payload.data()), msg.payload.size());
                    std::cout << std::endl;
                #endif
                callback(msg.payload, serial.getDeviceName().data());
            }
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

void DataLinkLayer::handleFrame(uint8_t type, const std::vector<uint8_t>& payload) {

}
