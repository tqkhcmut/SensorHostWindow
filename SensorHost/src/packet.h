#ifndef _packet_h_
#define _packet_h_

#include <stdint.h>

/* data type */
#define DATA_TYPE_MASK(x) (x & 0x1f)
#define BIT_ORDER_MASK(x) (x & 0xe0)

#define DATA_TYPE_BYTE		  0x01
#define DATA_TYPE_INT8		  0x02
#define DATA_TYPE_INT16		  0x03
#define DATA_TYPE_INT32		  0x04
#define DATA_TYPE_INT64		  0x05
#define DATA_TYPE_UINT16		0x06
#define DATA_TYPE_UINT32		0x07
#define DATA_TYPE_UINT64		0x08
#define DATA_TYPE_FLOAT		  0x09
#define DATA_TYPE_DOUBLE		0x0A

#define IS_DATA_TYPE_BYTE(x) 	  (DATA_TYPE_MASK(x) == TYPE_BYTE)
#define IS_DATA_TYPE_INT8(x) 	  (DATA_TYPE_MASK(x) == TYPE_INT8)
#define IS_DATA_TYPE_INT16(x) 	(DATA_TYPE_MASK(x) == TYPE_INT16)
#define IS_DATA_TYPE_INT32(x) 	(DATA_TYPE_MASK(x) == TYPE_INT32)
#define IS_DATA_TYPE_INT64(x) 	(DATA_TYPE_MASK(x) == TYPE_INT64)
#define IS_DATA_TYPE_UINT16(x) 	(DATA_TYPE_MASK(x) == TYPE_UINT16)
#define IS_DATA_TYPE_UINT32(x) 	(DATA_TYPE_MASK(x) == TYPE_UINT32)
#define IS_DATA_TYPE_UINT64(x) 	(DATA_TYPE_MASK(x) == TYPE_UINT64)
#define IS_DATA_TYPE_FLOAT(x) 	(DATA_TYPE_MASK(x) == TYPE_FLOAT)
#define IS_DATA_TYPE_DOUBLE(x) 	(DATA_TYPE_MASK(x) == TYPE_DOUBLE)

// for my thesis
#define DATA_TYPE_THESIS_DATA     0x0b
#define DATA_TYPE_THESIS_OUTPUT   0x0c
#define DATA_TYPE_THESIS_SIM      0x0d
#define DATA_TYPE_THESIS_TIME     0x0e
#define DATA_TYPE_THESIS_CLOCK    0x0f

#define IS_DATA_TYPE_THESIS_DATA(x) 	(DATA_TYPE_MASK(x) == DATA_TYPE_THESIS_DATA)
#define IS_DATA_TYPE_THESIS_OUTPUT(x) (DATA_TYPE_MASK(x) == DATA_TYPE_THESIS_OUTPUT)
#define IS_DATA_TYPE_THESIS_SIM(x) 	  (DATA_TYPE_MASK(x) == DATA_TYPE_THESIS_SIM)
#define IS_DATA_TYPE_THESIS_TIME(x) 	(DATA_TYPE_MASK(x) == DATA_TYPE_THESIS_TIME)
#define IS_DATA_TYPE_THESIS_CLOCK(x) 	(DATA_TYPE_MASK(x) == DATA_TYPE_THESIS_CLOCK)
/* data type */

/* byte order */
#define BIG_ENDIAN_BYTE_ORDER		  0x80
#define LITTLE_ENDIAN_BYTE_ORDER	0xc0

#define IS_BIG_ENDIAN_BYTE_ORDER(x)     (BIT_ORDER_MASK(x) == BIG_ENDIAN_BYTE_ORDER)
#define IS_LITTLE_ENDIAN_BYTE_ORDER(x)  (BIT_ORDER_MASK(x) == LITTLE_ENDIAN_BYTE_ORDER)
/* byte order */

/* devices type */
#define DEV_TYPE_MASK(x)    (x & 0xf0)
#define DEV_NUMBER_MASK(x)  (x & 0x0f)
#define	DEV_NUMBER_UNKNOWN	0x0f
#define	DEV_UNKNOWN			0xf0

#define DEV_SENSOR_TEMPERATURE    0x10
#define DEV_SENSOR_ULTRA_SONIC    0x20
#define DEV_SENSOR_GAS            0x30
#define DEV_SENSOR_LIGTH          0x40
#define DEV_BUZZER                0x50
#define DEV_SIM900                0x60
#define DEV_RF                    0x70
#define DEV_BLUETOOTH             0x80
// special device for my thesis, it have all temperature, lighting, and gas sensors
#define DEV_MY_THESIS					0x90

#define IS_SENSOR_TEMPERATURE(x) 	((x & 0xf0) == DEV_SENSOR_TEMPERATURE)
#define IS_SENSOR_ULTRA_SONIC(x) 	((x & 0xf0) == DEV_SENSOR_ULTRA_SONIC)
#define IS_SENSOR_GAS(x) 			((x & 0xf0) == DEV_SENSOR_GAS)
#define IS_SENSOR_LIGTH(x) 			((x & 0xf0) == DEV_SENSOR_LIGTH)
#define IS_BUZZER(x) 				((x & 0xf0) == DEV_BUZZER)
#define IS_SIM900(x) 				((x & 0xf0) == DEV_SIM900)
#define IS_RF(x) 					((x & 0xf0) == DEV_RF)
#define IS_BLUETOOTH(x) 			((x & 0xf0) == DEV_BLUETOOTH)
#define IS_MY_THESIS(x) 			((x & 0xf0) == DEV_MY_THESIS)
/* devices type */

// default time polling
//#define TEMPERATURE_POLL_TIME
//#define ULTRA_SONIC_POLL_TIME
//#define GAS
//

/* command */
#define CMD_TYPE_CONTROL  0x01
#define CMD_TYPE_QUERY    0x02
#define CMD_TYPE_RESPONSE 0x03

#define CMD_TYPE_MASK(x)  (x & 0x0f)

// special commands for my thesis
#define CMD_CLOCK         0x10
#define CMD_TIME_POLLING  0x20
#define CMD_SMS           0x30
#define CMD_SENSORS_VALUE 0x40
#define CMD_OUTPUT        0x50
#define CMD_ID            0x60
#define CMD_UNIQUE_NUMBER 0x70

#define CMD_MASK(x)       (x & 0xf0)
/* command */

#define BROADCAST_ID	0xff
#define IS_BROADCAST_ID(x) (x == BROADCAST_ID)

struct Packet
{
//  uint8_t len;  // packet length
	uint8_t id;	// include type and order in BCD number
  uint8_t unique_number[4];
	uint8_t cmd;  // include command type and command id
	uint8_t data_type;  // specify data structure and bytes order
	uint8_t data[]; // checksum number on the data byte number data_type
};

struct ThesisData
{
	float TempC;
  float Gas;
  float Lighting;
};

struct ThesisOutput
{
  uint8_t Buzzer;     // enable or disable buzzer
  uint8_t Speaker;    // enable or disable speaker
  uint8_t Relay;      // enable or disable relay
};

struct ThesisSIM
{
  uint8_t tar_num_len;  // target phone number length
  uint8_t tar_num[13];  // target phone numbers
  uint8_t sms;          // sms enable/disable
  uint8_t sms_msg[50];  // sms message content
};

struct ThesisTime
{
  uint32_t time_poll;
};

struct ThesisClock
{
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t day_of_week;
  uint8_t day;
  uint8_t month;
  uint8_t year;
  uint8_t padding;
};

struct Thesis
{
  struct ThesisData   _data;
  struct ThesisOutput _output;
  struct ThesisSIM    _sim;
  struct ThesisTime   _time_poll;
  // clock stored in RTC module
};


extern uint8_t getPacketLength(char * packet);
extern uint8_t getTypeLength(uint8_t data_type);
extern uint8_t checksum(char * packet);


#endif
