#pragma once

#include <string>
#include <stdint.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <iostream> 
#include <string.h>
#include <poll.h>

class SerialPort {
    public: 
        SerialPort(const std::string& device, int baudrate);
        ~SerialPort();
        bool openPort();
        void closePort();
        std::string getDeviceName();

        ssize_t writeBytes(const uint8_t* data, size_t size);
        ssize_t readBytes(uint8_t* buffer, size_t size);

    private:
        int fd;
        int baudrate;
        std::string device;
        struct termios tty;
        pollfd pfd;
};