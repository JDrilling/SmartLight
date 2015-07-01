#ifndef NRF_H
#define NRF_H

//*-----------------------PI------------------------*//
#ifdef __linux__
#include <iostream>
#include <string>
#include <cstring>
#include <bcm2835.h>
#include <bitset>
#include <unistd.h>


//Type conversions
#define byte unsigned char
#define String std::string

#define OUTPUT  BCM2835_GPIO_FSEL_OUTP
#define INPUT   BCM2835_GPIO_FSEL_INPT

// Pi Pins - - - - Hardware pin#
#define CE    RPI_GPIO_P1_22    //22
#define CSN   RPI_GPIO_P1_18    //18
#define IRQ   RPI_GPIO_P1_16    //16
#define MOSI  RPI_GPIO_P1_19    //19
#define MISO  RPI_GPIO_P1_21    //21
#define SCK   RPI_GPIO_P1_23    //23


void digitalWrite(int pin, int value);
void pinMode(int pin, int mode);

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
#define maxPayloadLength    (maxPacketLength - headerLength)

#define flushTXCMD      0b11100001
#define flushRXCMD      0b11100010
#define sendPayloadCMD  0b10100000
#define readPayloadCMD  0b01100001


#define firstPipeAddress 0b00010001
#define constPacketLength 32



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

    int getPacket(char * buff);
                                      
    void sendPacket(char * payload);
    void sendMessage(char * message, unsigned short destination);

    void transferByte(byte byteToWrite);

    void debugMessage(String messge);
    void test();

};
#endif


