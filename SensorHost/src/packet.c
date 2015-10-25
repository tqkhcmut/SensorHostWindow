#include "packet.h"


int getTypeLength(unsigned char type)
{
	switch (type & 0x0f)
	{
	case TYPE_BYTE:
		return 1; //sizeof(unsigned char);
		break;
	case TYPE_INT8:
		return 1; //sizeof(signed char);
		break;
	case TYPE_INT16:
		return 2; //sizeof(signed short);
		break;
	case TYPE_INT32:
		return 4; //sizeof(signed int);
		break;
	case TYPE_INT64:
		return 8; //sizeof(signed long);
		break;
	case TYPE_UINT16:
		return 2; //sizeof(unsigned short);
		break;
	case TYPE_UINT32:
		return 4; //sizeof(unsigned int);
		break;
	case TYPE_UINT64:
		return 8; //sizeof(unsigned long);
		break;
	case TYPE_FLOAT:
		return sizeof(float);
		break;
	case TYPE_DOUBLE:
		return sizeof(double);
		break;
	default:
		return 0;
		break;
	}
}

unsigned char checksum(char * packet)
{
	unsigned char checksum, packet_len, i;
	struct Packet * mypacket = (struct Packet *)packet;
	packet_len = getTypeLength(mypacket->data_type) + 3;
	for (i = 0; i < packet_len; i++)
		checksum += packet[i];
	checksum = !checksum + 1;
	return checksum;
}

