#ifndef _packet_h_
#define _packet_h_

/* data type */
#define TYPE_BYTE		0x01
#define TYPE_INT8		0x02
#define TYPE_INT16		0x03
#define TYPE_INT32		0x04
#define TYPE_INT64		0x05
#define TYPE_UINT16		0x06
#define TYPE_UINT32		0x07
#define TYPE_UINT64		0x08
#define TYPE_FLOAT		0x09
#define TYPE_DOUBLE		0x0A

#define IS_TYPE_BYTE(x) 	((x & 0x0f) == TYPE_BYTE)
#define IS_TYPE_INT8(x) 	((x & 0x0f) == TYPE_INT8)
#define IS_TYPE_INT16(x) 	((x & 0x0f) == TYPE_INT16)
#define IS_TYPE_INT32(x) 	((x & 0x0f) == TYPE_INT32)
#define IS_TYPE_INT64(x) 	((x & 0x0f) == TYPE_INT64)
#define IS_TYPE_UINT16(x) 	((x & 0x0f) == TYPE_UINT16)
#define IS_TYPE_UINT32(x) 	((x & 0x0f) == TYPE_UINT32)
#define IS_TYPE_UINT64(x) 	((x & 0x0f) == TYPE_UINT64)
#define IS_TYPE_FLOAT(x) 	((x & 0x0f) == TYPE_FLOAT)
#define IS_TYPE_DOUBLE(x) 	((x & 0x0f) == TYPE_DOUBLE)
/* data type */

/* byte order */
#define BIG_ENDIAN_BYTE_ORDER		0x10
#define LITTLE_ENDIAN_BYTE_ORDER	0x20

#define IS_BIG_ENDIAN_BYTE_ORDER(x) ((x & 0xf0) == BIG_ENDIAN_BYTE_ORDER)
#define IS_LITTLE_ENDIAN_BYTE_ORDER(x) ((x & 0xf0) == LITTLE_ENDIAN_BYTE_ORDER)
/* byte order */

/* Common mask */
#define DEV_TYPE_MASK(x)			(x & 0xf0)
#define DEV_NUMBER_MASK(x)			(x & 0x0f)
#define DATA_TYPE_MASK(x)			(x & 0x0f)
#define BIT_ORDER_MASK(x)			(x & 0xf0)


/* devices type */
#define DEV_SENSOR_TEMPERATURE		0x10
#define DEV_SENSOR_ULTRA_SONIC		0x20
#define DEV_SENSOR_GAS				0x30
#define DEV_SENSOR_LIGTH			0x40
#define DEV_BUZZER					0x50
#define DEV_SIM900					0x60
#define DEV_RF						0x70
#define DEV_BLUETOOTH				0x80
// special device for my thesis, it have all temperature, lighting, and gas sensors
#define DEV_MY_THESIS					0x90

#define DEV_BROADCAST				0xf0

#define IS_SENSOR_TEMPERATURE(x) 	(DEV_TYPE_MASK(x) == DEV_SENSOR_TEMPERATURE)
#define IS_SENSOR_ULTRA_SONIC(x) 	(DEV_TYPE_MASK(x) == DEV_SENSOR_ULTRA_SONIC)
#define IS_SENSOR_GAS(x) 			(DEV_TYPE_MASK(x) == DEV_SENSOR_GAS)
#define IS_SENSOR_LIGTH(x) 			(DEV_TYPE_MASK(x) == DEV_SENSOR_LIGTH)
#define IS_BUZZER(x) 				(DEV_TYPE_MASK(x) == DEV_BUZZER)
#define IS_SIM900(x) 				(DEV_TYPE_MASK(x) == DEV_SIM900)
#define IS_RF(x) 					(DEV_TYPE_MASK(x) == DEV_RF)
#define IS_BLUETOOTH(x) 			(DEV_TYPE_MASK(x) == DEV_BLUETOOTH)
#define IS_MY_THESIS(x) 			(DEV_TYPE_MASK(x) == DEV_MY_THESIS)
#define IS_BROADCAST_DEVICE(x)		(DEV_TYPE_MASK(x) == DEV_BROADCAST)
/* devices type */

// default time polling
//#define TEMPERATURE_POLL_TIME
//#define ULTRA_SONIC_POLL_TIME
//#define GAS
//

/* command */
#define CMD_CONTROL		0x01
#define CMD_QUERY		0x02
/* command */

#define BROADCAST_ID	0xff
#define IS_BROADCAST_ID(x) (x == BROADCAST_ID)

struct Packet
{
	unsigned char id;	// include type and order in BCD number
	unsigned char cmd;
	unsigned char data_type;
	unsigned char data[]; // checksum number on the data byte number data_type
};

struct ThesisData
{
	float temperature;
	float lighting;
	float gas;
};


int getTypeLength(unsigned char type);
unsigned char checksum(char * packet);


#endif
