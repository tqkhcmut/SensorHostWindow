#include "packet.h"


uint8_t getTypeLength(uint8_t data_type)
{
	switch (DATA_TYPE_MASK(data_type))
	{
	case DATA_TYPE_BYTE:
		return 1; //sizeof(uint8_t);
		break;
	case DATA_TYPE_INT8:
		return 1; //sizeof(signed char);
		break;
	case DATA_TYPE_INT16:
		return 2; //sizeof(signed short);
		break;
	case DATA_TYPE_INT32:
		return 4; //sizeof(signed int);
		break;
	case DATA_TYPE_INT64:
		return 8; //sizeof(signed long);
		break;
	case DATA_TYPE_UINT16:
		return 2; //sizeof(unsigned short);
		break;
	case DATA_TYPE_UINT32:
		return 4; //sizeof(unsigned int);
		break;
	case DATA_TYPE_UINT64:
		return 8; //sizeof(unsigned long);
		break;
	case DATA_TYPE_FLOAT:
		return sizeof(float);
		break;
	case DATA_TYPE_DOUBLE:
		return sizeof(double);
		break;
	case DATA_TYPE_THESIS_DATA:
		return sizeof(struct ThesisData);
		break;
	case DATA_TYPE_THESIS_OUTPUT:
		return sizeof(struct ThesisOutput);
		break;
	case DATA_TYPE_THESIS_SIM:
		return sizeof(struct ThesisSIM);
		break;
	case DATA_TYPE_THESIS_TIME:
		return sizeof(struct ThesisTime);
		break;
	case DATA_TYPE_THESIS_CLOCK:
		return sizeof(struct ThesisClock);
		break;
	default:
		return 0;
		break;
	}
}

uint8_t getPacketLength(char * packet)
{
	struct Packet * mypacket = (struct Packet *)packet;
  return sizeof(struct Packet) + getTypeLength(mypacket->data_type) + 1; // add 1 for checksum
}

uint8_t checksum(char * packet)
{
	uint8_t checksum, i;
//	packet_len = getTypeLength(mypacket->data_type) + 3;
	for (i = 0; i < getPacketLength(packet); i++)
		checksum += packet[i];
	checksum = !checksum + 1;
	return checksum;
}

