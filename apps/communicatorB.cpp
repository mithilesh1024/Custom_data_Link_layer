#include <termios.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include <mutex>
#include <thread>
#include <chrono>
#include <functional>

#include "SerialPort.hpp"
#include "DataLinkLayer.hpp"


std::mutex cout_mutex;
std::string input;

void printIncomingMessage(void* msg_ptr, void* sender_ptr) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::string msg = static_cast<char*>(msg_ptr);
    std::string sender = static_cast<char*>(sender_ptr);
    // Move cursor up one line, clear line, print message
    std::cout << "\r" << "\033[K"; // clear current input line
    std::cout <<sender_ptr << ":" << msg << std::endl;

    // Reprint the user input
    std::cout << "> " << input << std::flush;
}

void inputLoop(DataLinkLayer& dll) {
    std::vector<uint8_t> msg;
    while (true) {
        std::cout << "> " << std::flush;
        std::getline(std::cin, input);
        // process input...
        msg.clear();
        dll.convertDataToPayload(input, msg);
        dll.sendData(msg);
        std::cout << "You: " << input << std::endl;
    }
}

int main(int argc, char** argv) {
    std::string device = argv[1];
    int baudrate = std::stoi(argv[2]);

    SerialPort port(device, baudrate);
    port.openPort();
    DataLinkLayer dll(port, printIncomingMessage);
    std::thread inp(inputLoop, std::ref(dll));

    inp.join();

    return 0;
}