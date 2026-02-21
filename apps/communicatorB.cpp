#include <termios.h>
#include <iostream>
#include <string>

#include "SerialPort.hpp"

int main(int argc, char** argv) {
    std::string device = argv[1];
    int baudrate = std::stoi(argv[2]);

    SerialPort port(device, baudrate);
    port.openPort();

    size_t s = 6;
    uint8_t* data = new uint8_t[s];
    port.readBytes(data, s);
    std::cout<<"Printing data\n";
    for(int i=0;i<s;i++){
        std::cout<<data[i];
    }

    delete[] data;
    return 0;
}