#ifndef NRF_H
#define NRF_H

//*-----------------------PI------------------------*//
#ifdef __linux__
#include <iostream>
#include <string>
#include <cstring>
#include <wiringPiSPI.h> //TODO: Install this on PI
#include <wiringPi.h>

//Type conversions
#define byte unsigned char
#define String std::string

// Pi Pins - - - - Hardware pin#
#define CE   8      //24
#define CSN  25     //22
#define IRQ  24     //18
#define MOSI 10     //19
#define MISO 9      //21
#define SCK  11     //23

//*---------------------Arduino----------------------*//
#else

#define __arduino__ 1

#include <SPI.h>

#define CE   4
#define CSN  5
#define IRQ  2
#define MOSI 11
#define MISO 12
#define SCK  13

#endif

//*----------------------Common-----------------------*//
#include <limits.h>

#define destinationLength   2
#define sequenceLength      2
#define headerLength        (destinationLength + sequenceLength)
#define maxPacketLength     32
#define maxPayloadLength       (maxPacketLength - headerLength)

#define flushTXCMD      0b11100001
#define flushRXCMD      0b11100010
#define sendPayloadCMD  0b10100000
#define readPayloadCMD  0b01100001

#define firstPipeAddress 0b00110001
#define constPayloadLength 32


class NRF
{       
  public:    
    static unsigned short sequence;
    void init();
              
    void writeByte(byte address, byte byteToWrite);
    void writeBit(byte address, byte bitToWrite, byte value);

    void writeCMD(byte command);
                          
    byte readAddress(byte address);
    void clearInterrupts();

    String getPacket();
                                      
    void sendPacket(String payload);
    void sendMessage(String message, unsigned short destination);

    void transferByte(byte byteToWrite);

    void debugMessage(String message);

};
#endif


