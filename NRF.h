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
#include "SmartLightPacket.h"

#define flushTXCMD      0b11100001
#define flushRXCMD      0b11100010
#define sendPayloadCMD  0b10100000
#define readPayloadCMD  0b01100001
#define firstPipeAddress 0b00010001




class NRF
{       
  public:    
    static unsigned short sequence;

    //Initiates the NRF in RX Mode
    void init();

    //Stops SPI (Currently only for the pi SPI)
    void stop();

    //Writes A single Byte to the NRF
    //Changes CSN and writes to specific address
    void writeByte(byte address, byte byteToWrite);

    //Transfers a single byte over the SPI port.
    //Doesn't change CSN
    void transferByte(byte byteToWrite);

    //Overwrites a single bit on the NRF
    void writeBit(byte address, byte bitToWrite, byte value);

    //Writes one byte, but in the Command Format
    void writeCMD(byte command);

    //Reads a the given address from the NRF, returns the data in the address
    byte readAddress(byte address);

    //Clears the Interrupt outputs of the NRF
    void clearInterrupts();

    //Gets a packet of data. Currently 32 bytes long <destination 2B><sequence 2B><payload 28B>
    void getPacket(char * buff, int size);

    //Sends a 32 Byte Packet
    void sendPacket(char * packet);

    //Sends a message of any length. 
    //Message must by an NTCA
    void sendMessage(char * message, unsigned short destination);

    //Generic console output message
    void debugMessage(String messge);

    //Outputs several console messages to debug the status of the NRF
    void test();

};
#endif


