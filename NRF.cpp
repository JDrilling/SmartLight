#include "NRF.h"


unsigned short NRF::sequence = 0;

#ifdef __linux__
void pinMode(int pin, int mode){bcm2835_gpio_fsel(pin,mode);}
void digitalWrite(int pin, int val){
    bcm2835_gpio_write(pin,val);
}
#endif

void NRF::stop(){
#ifdef __linux__
  bcm2835_spi_end();
  bcm2835_close();
#endif
  return;
}


void NRF::init() {

#ifdef __linux__
  //Uncomment to Debug
  //bcm2835_set_debug(1);


  if(!bcm2835_init())
    std::cout << "Fail!!!!!!" << std::endl;
#endif

  //Initialize Pins
  pinMode(CE, OUTPUT);
  pinMode(CSN, OUTPUT);

#ifdef __arduino__
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCK, OUTPUT);

  //Set up SPI
  SPI.setBitOrder(MSBFIRST);
  //Keeps clock low until data is available
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  //Preset CE and CSN Pins to High.
  digitalWrite(CE, HIGH);
  digitalWrite(CSN, HIGH);

  dealay (10);

  //Start the SPI
  SPI.begin();
#endif
#ifdef __linux__
  digitalWrite(CE, HIGH);
  digitalWrite(CSN, HIGH);

  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);

  usleep(1000);
#endif

  
  debugMessage("SPI Set up");
  
  writeBit(0, 0, 1); //RX Mode
  std::cout << "RX" << std::endl;
  test();
  //writeBit(0, 0, 0); //Tx
  writeBit(0, 1, 1); //Power UP
  test();
  writeBit(0, 4, 1); //Disable MAX_RT Interrupt
  test();
  writeBit(0, 5, 1); //Disable TX_DS Interrupt
  test();
  //writeBit(0, 6, 0); //Enable RX_DR Interrupt
  //test();

  //Set Constant Payload Length
  writeByte(firstPipeAddress, constPacketLength);
  std::cout << "Payload Length" << std::endl;
  test();
  
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

#ifdef __linux__
  usleep(100);
  char temp[2];
  temp[0] = address | 0x20;
  temp[1] = byteToWrite;
  bcm2835_spi_transfern(temp, 2);

  usleep(100);
#endif
#ifdef __arduino__
  transferByte(address | 0x20);
  transferByte(byteToWrite);
#endif
  
  digitalWrite(CSN, HIGH);

#ifdef __linux__
  usleep(100);
#endif
  
  return;
}

byte NRF::readAddress(byte address) {
  byte readData;
  
  digitalWrite(CSN, LOW);
  
#ifdef __arduino__  
  transferByte(address);
  readData = SPI.transfer(0x00);
#endif

#ifdef __linux__
  usleep(100);

  char temp[2];
  temp[0] = (char)address;
  temp[1] = 0x00;
  bcm2835_spi_transfern(temp,2);
  readData = temp[1];

  usleep(100);
#endif
  
  digitalWrite(CSN, HIGH);

#ifdef __linux__
  usleep(100);
#endif
  
  return readData;
  }

void NRF::writeBit(byte address, byte bitToWrite, byte value) {
  //Get Previous Value
  byte prevRegValue = readAddress(address);

  if(value >= 1)
  {
    value = 0x01 << bitToWrite;
    value = value | prevRegValue;
  }
  else
  {
    value = 0x01 << bitToWrite;
    value = prevRegValue & ~value;
  }
  
  //Write new Value
  writeByte(address, value);
  
  return;
}

void NRF::clearInterrupts() {

    //Clear RT
    if (readAddress(0x07) & 0x10)
          writeBit(0x07, 4, 1);

      //Clear TX
    if (readAddress(0x07) & 0x20)
        writeBit(0x07, 5, 1);

        //Clear RX
    if (readAddress(0x07) & 0x40)
        writeBit(0x07, 6, 1);

    return;
}

String NRF::getPacket() {
  //debugMessage("Getting Packet...");
  
  //Get Packet Length
  byte packetLength = constPacketLength;//readAddress(0b1100000);
  String packet = "";


  //Initial Read CMD
  digitalWrite(CSN, LOW);

#ifdef __arduino__
  SPI.transfer(readPayloadCMD);

  //Read the payload
  for (byte i = 0; i < packetLength; i++)
    packet += SPI.transfer(0x00);
#endif
#ifdef __linux__
  transferByte(readPayloadCMD);

  char* cPacket = new char[packetLength];

  for (byte i = 0; i < packetLength; i++) cPacket[i] = 0x00;

  bcm2835_spi_transfern(cPacket, packetLength);

  packet = std::string(cPacket, cPacket + packetLength);
  delete cPacket;
#endif

  digitalWrite(CSN, HIGH);

  //Flush Anything left in the buffer
  writeCMD(flushRXCMD);

  writeBit(0x07, 6, 1);

  //debugMessage("Got Packet! ... ");
  //debugMessage(packet);
  return packet;
}

void NRF::sendMessage(char * message, unsigned short destination) {

  debugMessage("Sending...");
  debugMessage(message);

  int messageLength;
  char payload[constPacketLength + 1];

  //Can use this here. Message Must be null terminated.
  messageLength = strlen(message);

  //Divide the message into manageable packets
  for (int i = 0; i < messageLength; i += maxPayloadLength) {

    //Add the Header first
    payload[0] = (destination & 0xF0 >> 4);
    payload[1] = (destination & 0x0F); 
    payload[2] = (sequence & 0xF0 >> 4);
    payload[3] = (sequence & 0x0F);


    for(unsigned short j = 0; j < maxPayloadLength; j++)
    {

      //message Left >= 0
      if((messageLength - i - j) > 0)
        payload[4+j] = message[j+i];
      else
        payload[4+j] = '\0';
    }

    //Make Sure it's null Terminated
    //Even Though there is probably a null in there anyway
    payload[constPacketLength] = '\0';


#ifdef __linux__
    debugMessage("Payload...");
    for(unsigned short i = 0; i < constPacketLength; i++)
      std::cout << payload[i];

    std::cout << std::endl;
#endif

    sendPacket(payload);
  }

  
  return;
}

void NRF::sendPacket(char * packet) {

  unsigned int packetLength;

  packetLength = constPacketLength;//packet.length();

  //Get rid of any leftover data
  writeCMD(flushTXCMD);

  //Send the packet - Initial CMD
  digitalWrite(CSN, LOW);

  transferByte(sendPayloadCMD);


#ifdef __arduino__
  for (unsigned int i = 0; i < packetLength; i++)
    transferByte(packet[i]);

#endif
#ifdef __linux__
  std::cout << "Packet:  ";
  for(unsigned short i = 0; i < constPacketLength; i++)
    std::cout << packet[i];

  std::cout << std::endl;

  bcm2835_spi_transfern(packet, packetLength);

  std::cout << "Packet:   ";
  for(unsigned short i = 0; i < constPacketLength; i++)
    std::cout << packet[i];

  std::cout << std::endl;
#endif

  digitalWrite(CSN, HIGH);

  digitalWrite(CE, LOW);
#ifdef __linux__
  usleep(1000);
#endif
  writeBit(0,0,0);
#ifdef __linux__
  usleep(1000);
#endif
  digitalWrite(CE, HIGH);
#ifdef __linux__
  usleep(1000);
#endif
  writeBit(0,0,1);

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

#ifdef __arduino__
  delay(1);
#endif
#ifdef __linux__
  usleep(10000);
#endif
}

void NRF::transferByte(byte byteToWrite) {
#ifdef __arduino__
  SPI.transfer(byteToWrite);
#endif
#ifdef __linux__
  bcm2835_spi_transfer(byteToWrite);
#endif
}

void NRF::debugMessage(String message){
#ifdef __arduino__
  Serial.println(message);
#endif
#ifdef __linux__
  std::cout << message << std::endl;
#endif
}

void NRF::test(){
  #ifdef __linux__
  debugMessage("----------Test Begin----------");
  String sReadData;
  byte readData;

  debugMessage("Status Reg");
  readData = readAddress(0x07);
  std::bitset<8> x(readData);
  std::cout << x << std::endl;
  debugMessage(sReadData);

  debugMessage("Config Reg");
  readData = readAddress(0x00);
  std::bitset<8> y(readData);
  std::cout << y << std::endl;
  debugMessage(sReadData);

  debugMessage("----------Test End------------");
  #endif
}
