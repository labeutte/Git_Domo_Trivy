/******************************************************************************
 *  serieLin.h
 *  gestion liaison serie par USB
 ******************************************************************************/
#ifndef SERIELIN_H
#define SERIELIN_H

#include <iostream>
#include <string>     // std::string, std::to_string
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h>  //Used for UART
//#include <wiringSerial.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
using namespace std;

class serieLin {
private:
    char uartNom[20];
    int uartStream;
    char buffer[256]; /* Input buffer des réponses AT */
    string reponseAT;
public:
    serieLin();
    ~serieLin();
    void set_blocking(int fd, int should_block);
    int set_interface_attribs(int fd, int speed, int parity);
    void set_mincount(int fd, int vmin, int vtime);
    int openTTY(const char* port);
    void closeTTY(void);
    int writeTTY(char *tx_buffer, int lg_buffer);
    int writeImTTY(char *tx_buffer, int lg_buffer);
    int readTTY(char* rx_buffer, int nBytesToRead);
    int readTimeOut(char* rx_buffer, int nBytesToRead, int timeout);
    string sendAT(const char* ATcommand, const char* repAttendu, unsigned int timeout);
};

#endif // SERIELIN_H
