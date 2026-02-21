#include <termios.h>
#include <iostream>
#include <string>

#include "SerialPort.hpp"

int main(int argc, char** argv) {
    std::string device = argv[1];
    int baudrate = std::stoi(argv[2]);

    SerialPort port(device, baudrate);
    port.openPort();

    const char* msg = "Hello\n";
    port.writeBytes(reinterpret_cast<const uint8_t*>(msg), 6);

    return 0;
}