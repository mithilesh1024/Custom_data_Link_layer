#include "SerialPort.hpp"
 
SerialPort::SerialPort(const std::string& device, int baudrate) {
    this->device = device;
    this->baudrate = baudrate;
    this->fd = -1;
}

SerialPort::~SerialPort() {
    
}

std::string SerialPort::getDeviceName() {
    return this->device;
}

bool SerialPort::openPort() {
    fd = open(device.c_str(), O_RDWR | O_NOCTTY );
    if(fd < 0) {
        std::cout<<" Error in opening "<<device<<std::endl;
        return false;
    }

    this->pfd.fd = fd;
    this->pfd.events = POLLIN;

    struct termios tty;
    if(tcgetattr(fd, &tty) != 0) return false;

    cfsetispeed(&tty, baudrate);
    cfsetospeed(&tty, baudrate);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag = 0;

    tty.c_cc[VMIN]  = 1;  // return as soon as 1 byte is available
    tty.c_cc[VTIME] = 0;  // no timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0) return false;
    
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);

    return true;
}

void SerialPort::closePort() {
    if(fd >= 0)
        close(fd);
    fd = -1;
}

ssize_t SerialPort::writeBytes(const uint8_t* data, size_t size) {

    if(fd == -1) return 0;

    ssize_t res = write(fd, data, size);

    return res;
}

ssize_t SerialPort::readBytes(uint8_t* buffer, size_t size) {

    ssize_t bytesRead = 0;
    if(fd == -1) return 0;
    int ret = poll(&pfd, 1, 0);
    if(ret == -1){
        std::cout<<"Error in polling";
    }else{
        bytesRead = read(fd, buffer, size);
        if(bytesRead < 0) {
            perror("read");
            return 0;
        }
    }
    
    return bytesRead;
}

