#include <termios.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include <mutex>
#include <thread>
#include <chrono>
#include <functional>
#include <poll.h>

#include "SerialPort.hpp"
#include "DataLinkLayer.hpp"

std::mutex cout_mutex;
std::string input;

void printIncomingMessage(std::vector<uint8_t> msg, void* sender_ptr) {
    
    std::string sender = static_cast<char*>(sender_ptr);
    // Move cursor up one line, clear line, print message
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\r" << "\033[K"; // clear current input line
        std::cout <<sender << ":";
        for(auto m : msg) {
            if(isprint(m)) std::cout<<static_cast<char>(m);
            else    std::cout<<"..";
        }
        std::cout<<std::endl;
        // Reprint the user input
        std::cout << "> " << input << std::flush;
    }
}

void inputLoop(DataLinkLayer& dll) {
    std::vector<uint8_t> msg;
    char c;
    pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    while (true) {
        input = "";
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "> " << std::flush;
        }
        
        // This loop stays alive until 'Enter' is hit
        while (true) {
            // Wait indefinitely for a keypress (-1)
            int ret = poll(&pfd, 1, -1); 
            
            if (ret > 0 && (pfd.revents & POLLIN)) {
                read(STDIN_FILENO, &c, 1);
                
                if (c == '\n' || c == '\r') {
                    {
                        std::lock_guard<std::mutex> lock(cout_mutex);
                        std::cout << "\r\033[K" << std::flush; // clear current input line
                    }
                    break; // Exit character-gathering loop
                } else if (c == 127 || c == 8) { // Handle backspace
                    if (!input.empty()) {
                        {
                            std::lock_guard<std::mutex> lock(cout_mutex);
                            input.pop_back();
                            std::cout << "\b \b" << std::flush;
                        }
                    }
                } else {
                    input += c;
                    // Echo character manually since ECHO is off in your termios
                    std::cout << c << std::flush; 
                }
            }
        }
        // process input...
        msg.clear();
        dll.convertDataToPayload(input, msg);
        dll.sendData(msg);
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "You: " << input << std::endl;
        }
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