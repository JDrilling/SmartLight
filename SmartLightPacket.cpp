#include "SmartLightPacket.h"

unsigned short slGetDestination(char cPacket[]){
	return ((unsigned short)cPacket[0] << 8) + (unsigned short)cPacket[1];
}

unsigned short slGetSequence(char cPacket[]){
	return ((unsigned short)cPacket[2] << 8) + (unsigned short)cPacket[3];
}

void slGetPayload(char payload[], char cPacket[], unsigned int payloadlength){
	if(payloadlength >= maxPayloadLength)
		memcpy(payload, &cPacket[4], maxPayloadLength);
	return;
}
