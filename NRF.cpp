#include "NRF.h"

unsigned short NRF::sequence = 0;

void NRF::init() {

#ifdef __linux__
  wiringPiSetupGpio();
#endif

  //Initialize Pins
  pinMode(CE, OUTPUT);
  pinMode(CSN, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCK, OUTPUT);

#ifdef __arduino__
  //Set up SPI
  SPI.setBitOrder(MSBFIRST);
  //Keeps clock low until data is available
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
#endif

  //Preset CE and CSN Pins to High.
  digitalWrite(CE, HIGH);
  digitalWrite(CSN, HIGH);

#ifdef __arduino__
  //Start the SPI
  SPI.begin();
#endif
#ifdef __linux__
  wiringPiSPISetup(0, 8000000);
#endif
  
  debugMessage("SPI Set up");
  
  writeBit(0, 0, 1); //RX Mode
  writeBit(0, 1, 1); //Power UP
  writeBit(0, 4, 1); //Disable MAX_RT Interrupt
  writeBit(0, 5, 1); //Disable TX_DS Interrupt
  //writeBit(0, 6, 0); //Enable RX_DR Interrupt
  //Set Constant Payload Length
  writeByte(firstPipeAddress, constPayloadLength);

  
  //writeBit(0x1D, 2, 1); //Enable DPL
  
  //Flush RX
  writeCMD(flushRXCMD);
  
  //Flush TX
  writeCMD(flushTXCMD);
  
  clearInterrupts();
  debugMessage("Init Finish");
  return;
}

void NRF::writeByte(byte address, byte byteToWrite) {

  digitalWrite(CSN, LOW);
  
  transferByte(address | 0x20);
  transferByte(byteToWrite);
  
  digitalWrite(CSN, HIGH);
  
  return;
}

byte NRF::readAddress(byte address) {
  byte readData;
  
  digitalWrite(CSN, LOW);
  
  transferByte(address);
  
#ifdef __arduino__
  readData = SPI.transfer(0x00);
#endif
#ifdef __linux__
  wiringPiSPIDataRW(0, &readData, 1);
#endif
  
  digitalWrite(CSN, HIGH);
  
  return readData;
  }

void NRF::writeBit(byte address, byte bitToWrite, byte value) {
  //Get Previous Value
  byte prevRegValue = readAddress(address);
  value = value & 0x01;
  value <<= bitToWrite;

  if(value == 1)
    value = value | prevRegValue;
  else
    value = prevRegValue & ~value;
  
  //Write new Value
  writeByte(address, value);
  
  return;
}

void NRF::clearInterrupts() {

    //Clear RT
    if (readAddress(0x07) & 0x08)
          writeBit(0x07, 4, 1);

      //Clear TX
    if (readAddress(0x07) & 0x10)
        writeBit(0x07, 5, 1);

        //Clear RX
    if (readAddress(0x07) & 0x20)
        writeBit(0x07, 6, 1);

    return;
}

String NRF::getPacket() {
  debugMessage("Getting Packet...");
  
  //Get Packet Length
  byte packetLength = constPayloadLength;//readAddress(0b1100000);
  String packet = "";


  //Initial Read CMD
  digitalWrite(CSN, LOW);

#ifdef __arduino__
  Serial.println(packetLength);
  SPI.transfer(readPayloadCMD);

  //Read the payload
  for (byte i = 0; i < packetLength; i++)
    packet += SPI.transfer(0x00);
#endif
#ifdef __linux__
  unsigned char cmd[1] = {readPayloadCMD};

  wiringPiSPIDataRW(0, cmd, 1);
  unsigned char* cPacket = new unsigned char[packetLength];

  for (byte i = 0; i < packetLength; i++) cPacket[i] = 0x00;

  wiringPiSPIDataRW(0, cPacket, packetLength);

  packet = std::string(cPacket, cPacket + packetLength);
  delete cPacket;
#endif

  digitalWrite(CSN, HIGH);

  //Flush Anything left in the buffer
  writeCMD(flushRXCMD);

  debugMessage("Got Packet! ... ");
  debugMessage(packet);
  return packet;
}

void NRF::sendMessage(String message, unsigned short destination) {

  debugMessage("Sending...");
  debugMessage(message);

  unsigned int messageLeft;
  unsigned int messageLength;
  String payload = "";

  messageLength = message.length();
  messageLeft = messageLength;

  //Divide the message into manageable packets
  for (unsigned int i = 0; i < messageLength; i += maxPayloadLength) {

#ifdef __arduino__
    //Add the Header first
    payload = String(destination) + String(sequence);

    if (messageLeft >= maxPayloadLength)
      payload += message.substring(i, i + maxPayloadLength);
    else //If there isn't a full playload left
      payload += message.substring(i);
#endif

#ifdef __linux__
    payload = std::to_string(destination);
    payload += std::to_string(sequence);

    if (messageLeft >= maxPayloadLength)
      payload += message.substr(i, i + maxPayloadLength);
    else //If there isn't a full playload left
      payload += message.substr(i);
#endif

    debugMessage("Payload...");
    debugMessage(payload);

    sendPacket(payload);
  }

  return;
}

void NRF::sendPacket(String packet) {

  unsigned int packetLength;

  packetLength = constPayloadLength;//packet.length();

  //Get rid of any leftover data
  writeCMD(flushTXCMD);

  //Send the packet - Initial CMD
  digitalWrite(CSN, LOW);

  transferByte(sendPayloadCMD);

  debugMessage("Packet To send...");
  debugMessage(packet);

#ifdef __arduino__
  for (unsigned int i = 0; i < packetLength; i++)
    SPI.transfer(packet[i]);

#endif
#ifdef __linux__
  unsigned char * cPacket = new unsigned char[packetLength + 1];
  std::strcpy((char *) cPacket, packet.c_str());
  wiringPiSPIDataRW(0, cPacket, packetLength);
#endif

  digitalWrite(CSN, HIGH);

  digitalWrite(CE, LOW);
  writeBit(0,0,0);
  digitalWrite(CE, HIGH);
  digitalWrite(CE, LOW);
  writeBit(0,0,1);
  digitalWrite(CE, HIGH);

  if (sequence == USHRT_MAX)
    sequence = 0;
  else
    sequence++;

  return;
}

void NRF::writeCMD(byte command) {
  digitalWrite(CSN, LOW);
  transferByte(command);
  digitalWrite(CSN, HIGH);
}

void NRF::transferByte(byte byteToWrite) {
#ifdef __arduino__
  SPI.transfer(byteToWrite);
#endif
#ifdef __linux__
  wiringPiSPIDataRW(0, &byteToWrite, 1);
#endif
}

void NRF::debugMessage(String message){
#ifdef __arduino__
  Serial.println(message);
#endif
#ifdef __linux__
  std::cout << messge << std::endl;
#endif
}

