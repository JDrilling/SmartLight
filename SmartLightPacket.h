#ifndef SMARTLIGHTPACKET_H
#define SMARTLIGHTPACKET_H

#include "NRF.h"

#define destinationLength   2
#define sequenceLength      2
#define headerLength        (destinationLength + sequenceLength)
#define constPacketLength 32
#define maxPacketLength     32
#define maxPayloadLength    (maxPacketLength - headerLength)

unsigned short slGetDestination(char cPacket[]);
unsigned short slGetSequence(char cPacket[]);
void slGetPayload(char payload[], char cPacket[], unsigned int payloadlength = maxPayloadLength);

#endif
