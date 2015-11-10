/*
 * sensor.c
 *
 *  Created on: Sep 17, 2015
 *      Author: kieu
 */

#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
// No translation needed for big endian system
#define Swap2Bytes(val) val
#define Swap4Bytes(val) val
#define Swap8Bytes(val) val
#else
// Swap 2 byte, 16 bit values:
#define Swap2Bytes(val) \
		( (((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00) )

// Swap 4 byte, 32 bit values:
#define Swap4Bytes(val) \
		( (((val) >> 24) & 0x000000FF) | (((val) >>  8) & 0x0000FF00) | \
				(((val) <<  8) & 0x00FF0000) | (((val) << 24) & 0xFF000000) )

// Swap 8 byte, 64 bit values:
#define Swap8Bytes(val) \
		( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | \
				(((val) >> 24) & 0x0000000000FF0000) | (((val) >>  8) & 0x00000000FF000000) | \
				(((val) <<  8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | \
				(((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000) )
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "devices.h"
#include "serial.h"
#include "ishare.h"
#include "raspi_ext.h"

#ifdef __linux
#include <unistd.h>
#include "wiringPi/wiringPi.h"
#endif

#include <pthread.h>
	
#include "../queue.h"
#include "../register.h"
#include "../db_com.h"

#define DEV_HOST_NUMBER 4 // 4 USB interfaces

pthread_t polling_thread[DEV_HOST_NUMBER];
struct Device dev_host[DEV_HOST_NUMBER];

// protect device host access
pthread_mutex_t device_control_access = PTHREAD_MUTEX_INITIALIZER;
// protect serial access
pthread_mutex_t serial_access = PTHREAD_MUTEX_INITIALIZER;

#if DATABASE
#include <sqlite3.h>
	
// Pointer to Sqlite3 DB - used to access DB when open
sqlite3 *db = NULL;
// Path to DB file - same dir as this program's executable
const char *dbPath = "../raspi_sensor.db";
// DB Statement handle - used to run SQL statements
sqlite3_stmt *stmt = NULL;

pthread_mutex_t db_token = PTHREAD_MUTEX_INITIALIZER;

/* 
CREATE TABLE [sensor_types] (
[sensor_types_id] INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT,
[name] NVARCHAR(200)  NOT NULL,
[description] nvarCHAR(250)  NULL
);

CREATE TABLE [sensor_values] (
[sensor_values_id] INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT,
[sensors_id] intEGER  NOT NULL,
[sensor_value] FLOAT  NOT NULL,
[timestamp] FLOAT  NOT NULL,
[created] TIMESTAMP  NOT NULL
);

CREATE TABLE [sensors] (
[sensors_id] INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT,
[sensor_types_id] intEGER  NOT NULL,
[name] nvarCHAR(250)  NOT NULL,
[code] nvaRCHAR(50)  NOT NULL,
[data_symbol] nvaRCHAR(50)  NULL,
[sample_time] intEGER  NULL,
[sample_speed] intEGER  NULL,
[unit] nvaRCHAR(200)  NULL
);
*/

// some speacial informations for each sensors

// Ultra Sonic
const char * UltraSonic_name = "Ultra Sonic";
const char * UltraSonic_description = "Measure distance use ultra sonic waveform.";
const char * UltraSonic_code = "US";
const char * UltraSonic_symbol = "US";
const char * UltraSonic_unit = "cm";

const char * sensor_types = "INSERT INTO sensor_types(sensor_types_id, name, description) VALUES(?, ?, ?)";
const char * sensor_values = "INSERT INTO sensor_values(sensor_values_id, sensors_id, sensor_value, measured_timestamp, created) VALUES(?, ?, ?, ?, ?)";
const char * sensor_values_short = "INSERT INTO sensor_values(sensor_value, measured_timestamp) VALUES(?, ?)";
const char * sensors = "INSERT INTO sensors(sensors_id, sensor_types_id, name, code, data_symbol, sample_time, sample_speed, unit) VALUES(?, ?, ?, ?, ?, ?, ?, ?)";

int DB_Record_sensor_types(int sensor_types_id, char *name, char * description) 
{
	if (pthread_mutex_trylock(&db_token) == 0)
	{	
		sqlite3_prepare_v2(db, sensor_types, strlen(sensor_types), &stmt, NULL);
		sqlite3_bind_int(stmt, 1, sensor_types_id);
		sqlite3_bind_text(stmt, 2, name, strlen(name), 0);
		sqlite3_bind_text(stmt, 3, description, strlen(description), 0);	

		sqlite3_step(stmt);  // Run SQL INSERT

		sqlite3_reset(stmt); // Clear statement handle for next use
	
		pthread_mutex_unlock(&db_token);
	}
	else
	{
#if DEVICE_DEBUG
		printf("Cannot access db token.\n");
#endif
			return - 1;
	}
	return 0;
}
int DB_Record_sensor_values(int sensor_values_id, int sensors_id, float sensor_value, float measured_timestamp, unsigned int created)
{
#if DEVICE_DEBUG 
	printf("DB_Record_sensor_values.\n");
#endif
	if (pthread_mutex_trylock(&db_token) == 0)
	{	
		sqlite3_prepare_v2(db, sensor_values, strlen(sensor_values), &stmt, NULL);
		sqlite3_bind_int(stmt, 1, sensor_values_id);
		sqlite3_bind_int(stmt, 2, sensors_id);
		sqlite3_bind_double(stmt, 3, sensor_value);
		sqlite3_bind_double(stmt, 4, measured_timestamp);
		sqlite3_bind_int(stmt, 5, created);

		sqlite3_step(stmt);  // Run SQL INSERT

		sqlite3_reset(stmt); // Clear statement handle for next use

		pthread_mutex_unlock(&db_token);
	}
	else
	{
#if DEVICE_DEBUG 
		printf("Cannot access db token.\n");
#endif
			return - 1;
	}
	return 0;
}
int DB_Record_sensor_values_short(float sensor_value, float measured_timestamp)
{
#if DEVICE_DEBUG 
	printf("DB_Record_sensor_values_short.\n");
#endif
	if (pthread_mutex_trylock(&db_token) == 0)
	{	
		if (sqlite3_prepare(db, sensor_values_short, strlen(sensor_values_short), &stmt, NULL) != SQLITE_OK)
		{
#if DEVICE_DEBUG 
			printf("DB_Record_sensor_values_short: error occur while sqlite3_prepare.\n");
#endif
			return - 1;
		}
		if (sqlite3_bind_double(stmt, 1, sensor_value) != SQLITE_OK)
		{
#if DEVICE_DEBUG 
			printf("DB_Record_sensor_values_short: error occur while sqlite3_bind_double.\n");
#endif
			return -1;
		}
		if (sqlite3_bind_double(stmt, 2, measured_timestamp) != SQLITE_OK)
		{
#if DEVICE_DEBUG 
			printf("DB_Record_sensor_values_short: error occur while sqlite3_bind_double.\n");
#endif
			return -1;
		}

		sqlite3_step(stmt);  // Run SQL INSERT

		sqlite3_reset(stmt); // Clear statement handle for next use

		pthread_mutex_unlock(&db_token);
	}
	else
	{
#if DEVICE_DEBUG 
		printf("Cannot access db token.\n");
#endif
			return - 1;
	}
	return 0;
}
int DB_Record_sensors(int sensors_id, int sensor_types_id, char *name, char * code, char * data_symbol, int sample_time, int sample_speed, char * unit) 
{
#if DEVICE_DEBUG
	printf("DB_Record_sensors.\n");
#endif
	if (pthread_mutex_trylock(&db_token) == 0)
	{	
	sqlite3_prepare(db, sensors, strlen(sensors), &stmt, NULL);
	sqlite3_bind_int(stmt, 1, sensors_id);
	sqlite3_bind_int(stmt, 2, sensor_types_id);
	sqlite3_bind_text(stmt, 3, name, strlen(name), 0);
	sqlite3_bind_text(stmt, 4, code, strlen(code), 0);	
	sqlite3_bind_text(stmt, 5, data_symbol, strlen(data_symbol), 0);	
	sqlite3_bind_int(stmt, 6, sample_time);
	sqlite3_bind_int(stmt, 7, sample_speed);
	sqlite3_bind_text(stmt, 8, unit, strlen(unit), 0);	

	sqlite3_step(stmt);  // Run SQL INSERT

	sqlite3_reset(stmt); // Clear statement handle for next use

		pthread_mutex_unlock(&db_token);
	}
	else
	{
#if DEVICE_DEBUG
		printf("Cannot access db token.\n");
#endif
			return - 1;
	}
	return 0;
}
int DB_IsExist_sensors(int sensors_id, int sensor_types_id, char *name, char * code, char * data_symbol, int sample_time, int sample_speed, char * unit) 
{
	
	return 0;
}
int DB_IsExist_sensor_types(int sensor_types_id, char *name, char * description) 
{
	
	return 0;
}
#endif  // DATABASE

int sendControl(struct Device dev)
{
	int packet_len = 3 + getTypeLength(dev.data_type);
	struct Packet * packet = malloc(packet_len);

	packet->id = dev.number | dev.type;
	packet->cmd = CMD_CONTROL;
#if __BYTE_ORDER == __BIG_ENDIAN
	packet->data_type = (dev->data_type & 0x0f) | BIG_ENDIAN_BYTE_ORDER;
#else
	packet->data_type = (dev.data_type & 0x0f) | LITTLE_ENDIAN_BYTE_ORDER;
#endif
	memcpy(packet->data, dev.data, packet_len - 3);
	// add checksum byte
	*(((char *)packet) + packet_len) = checksum((char *)packet);

	Serial_SendMultiBytes((unsigned char *) packet, packet_len);
	free(packet);
	return 0;
}

#if DEVICE_DEBUG
int DeviceInfo(struct Device * dev)
{
	int i;
	printf("Device information.\n");
	printf("Device Type: %02X.\n", dev->type);
	printf("Device Number: %02X.\n", dev->number);
	printf("Device Data Type: %02X.\n", dev->data_type);
	printf("Device Data: ");
	for (i = 0; i < getTypeLength(dev->data_type); i++)
	{
		printf("%02X ", dev->data[i]);
	}
	printf("\n");
	return i;
}
#endif

int queryData(struct Device * dev)
{
	int packet_len;
	struct Packet * packet = NULL;

	if (dev->data == NULL)
	{
		if (IS_MY_THESIS(dev->type))
		{
			dev->data = malloc(sizeof (struct ThesisData));
			memset(dev->data, 0, sizeof (struct ThesisData));
		}
		else
		{
			dev->data = malloc(getTypeLength(dev->data_type));
			memset(dev->data, 0, getTypeLength(dev->data_type));
		}
	}

#if DEVICE_DEBUG
	printf("Query Device: \n");
	DeviceInfo(dev);
#endif

	if (IS_MY_THESIS(dev->type))
	{
		packet_len = 3 + sizeof (struct ThesisData);
		packet = malloc(packet_len + 1);
	}
	else
	{
		packet_len = 3 + getTypeLength(dev->data_type);
		packet = malloc(packet_len + 1);
	}

	packet->id = dev->number | dev->type;
	packet->cmd = CMD_QUERY;
#if __BYTE_ORDER == __BIG_ENDIAN
	packet->data_type = (dev->data_type & 0x0f) | BIG_ENDIAN_BYTE_ORDER;
#else
	packet->data_type = (dev->data_type & 0x0f) | LITTLE_ENDIAN_BYTE_ORDER;
#endif
	memcpy(packet->data, dev->data, packet_len - 3);
	// add checksum byte
	*(((char *)packet) + packet_len) = checksum((char *)packet);

#if DEVICE_DEBUG
	printf("Query Packet: ");
	int i;
	for (i = 0; i < packet_len + 1; i++)
	{
		printf("%02X ", *((unsigned char *) packet + i));
	}
	printf("Checksum: %02X.\n", *(((char *)packet) + packet_len));
	printf("\n");
#endif

	Serial_SendMultiBytes((unsigned char *) packet, packet_len + 1);
	// sleep 5ms for timeout
	usleep(5000);
	packet_len = Serial_Available();
	if (packet_len)
	{
		Serial_GetData((char *) packet, packet_len);
		// checksum check
		if (checksum((char *)packet) != packet->data[getTypeLength(packet->data_type)])
		{
#if DEVICE_DEBUG
			printf("Wrong checksum.\n");
#endif
			// what should to do now?
		}

#if DEVICE_DEBUG
		printf("Received Packet: ");
		int i;
		for (i = 0; i < packet_len + 1; i++)
		{
			printf("%02X ", *((unsigned char *) packet + i));
		}
		printf("Checksum: %02X.\n", *(((char *)packet) + packet_len));
		printf("\n");
#endif

		if (dev->data_type != DATA_TYPE_MASK(packet->data_type))
		{
			free(dev->data);
			if (IS_MY_THESIS(packet->data_type & 0x0f))
			{
				dev->data = malloc(sizeof(struct ThesisData));
			}
			else
			{
				dev->data = malloc(getTypeLength(packet->data_type & 0x0f));
			}
		}

		dev->data_type = DATA_TYPE_MASK(packet->data_type);
		dev->type = DEV_TYPE_MASK(packet->id);
		dev->number = DEV_NUMBER_MASK(packet->id);
		if (IS_MY_THESIS(packet->id))
		{
			dev->polling_control.time_poll_ms = 500;
			memcpy(dev->data, packet->data, sizeof (struct ThesisData));
		}
		else
		{
			memcpy(dev->data, packet->data, getTypeLength(packet->data_type));
		}
		if (packet != NULL)
			free(packet);
		return packet_len;
	}
	else
	{
		if (packet != NULL)
			free(packet);
		return 0;
	}
}

void * DevicePolling(void * host_number) // thread
{
	unsigned char poll_en = 0;
	unsigned int time_poll = 0;
	unsigned char destroy = 0;
	int host = (int) host_number;
	unsigned char trying_time = 0, dev_disconnect_try_time = 3;
	float_struct_t my_float;

	if (host >= DEV_HOST_NUMBER)
	{
		printf("Host number not valid.\r\nIt should be lester than %d.\r\n", DEV_HOST_NUMBER);
		printf("Thread exiting.\r\n");
		pthread_exit(NULL);
	}

	printf("Thread: %d start with host: %d.\n",
			(int)polling_thread[host], host);

	while(1)
	{
		if (pthread_mutex_trylock(&device_control_access) == 0)
		{
			poll_en = dev_host[host].polling_control.enable;
			time_poll = dev_host[host].polling_control.time_poll_ms * 1000;
			destroy = dev_host[host].polling_control.destroy;
			pthread_mutex_unlock(&device_control_access);
		}
		else
		{
			printf("Thread: %d. host: %d. Fail to access device control.\n",
					(int)polling_thread[host], host);
			usleep(1000);
		}

		if (destroy)
		{
			printf("Thread: %d. host: %d. Destroying.\n",
					(int)polling_thread[host], host);
			pthread_exit(NULL);
		}

		if (poll_en)
		{
			if (dev_host[host].type != DEV_UNKNOWN) // already known device type
			{
				trying_time = 0;
				while (pthread_mutex_trylock(&serial_access) != 0)
				{
					usleep(1000);
					trying_time ++;
					if (trying_time > 10)
						break;
				}
				if (trying_time > 10)
				{
#if DEVICE_DEBUG
					printf("Thread: %d. host: %d. Fail to access serial port.\n",
							(int)polling_thread[host], host);
#endif
				}
				else
				{
					RaspiExt_Pin_Hostx_Active(host);
					if (queryData(&dev_host[host]))
					{
#if DEVICE_DEBUG
						printf("Thread: %d. host: %d. Got data from device.\n",
								(int)polling_thread[host], host);
						DeviceInfo(&dev_host[host]);
#endif
						dev_disconnect_try_time = 3;
					}
					else
					{
#if DEVICE_DEBUG
						printf("Thread: %d. host: %d. No device here.\n",
								(int)polling_thread[host], host);
#endif
						if (dev_disconnect_try_time == 0)
						{						
						// TODO: unregister this device
							unsigned char reg_id = dev_host[host].number | dev_host[host].type;
							printf("Thread: %d. host: %d. Unregister device %X.\n",
								(int)polling_thread[host],
								host, 
								reg_id);	
							UnRegisterID(&reg_id);
							dev_host[host].type = DEV_UNKNOWN;
							dev_host[host].number = DEV_NUMBER_UNKNOWN;
						}
						else
						{
							dev_disconnect_try_time--;
						}
					}
					RaspiExt_Pin_Hostx_Inactive(host);
					pthread_mutex_unlock(&serial_access);
				}

				switch (dev_host[host].type)
				{
				case DEV_SENSOR_TEMPERATURE:
					RaspiExt_LED_Hostx_Config(LED_MODE_TOGGLE, 1000, host);
					
					if (IS_BIG_ENDIAN_BYTE_ORDER(dev_host[host].data_type))
					{
						my_float.f_byte[0] = dev_host[host].data[3];
						my_float.f_byte[1] = dev_host[host].data[2];
						my_float.f_byte[2] = dev_host[host].data[1];
						my_float.f_byte[3] = dev_host[host].data[0];
					}
					else
					{
						my_float.f_byte[0] = dev_host[host].data[0];
						my_float.f_byte[1] = dev_host[host].data[1];
						my_float.f_byte[2] = dev_host[host].data[2];
						my_float.f_byte[3] = dev_host[host].data[3];
					}
					printf("Thread: %d. host: %d. Temperature: %0.3f.\n",
						(int)polling_thread[host],
						host,
						my_float.f);

					// adjust time polling
					if (pthread_mutex_trylock(&device_control_access) == 0)
					{
						dev_host[host].polling_control.time_poll_ms = 50;
					}
					else
					{
						printf("Thread: %d. host: %d. Fail to access device control.\n",
							(int)polling_thread[host],
							host);
					}

					// save to shared memory

					break;
				case DEV_SENSOR_ULTRA_SONIC:
					RaspiExt_LED_Hostx_Config(LED_MODE_TOGGLE, 100, host);
					
					my_float.f_byte[0] = dev_host[host].data[3];
					my_float.f_byte[1] = dev_host[host].data[2];
					my_float.f_byte[2] = dev_host[host].data[1];
					my_float.f_byte[3] = dev_host[host].data[0];
					printf("Thread: %d. host: %d. Distance: %0.3f.\n",
						(int)polling_thread[host],
						host,
						my_float.f);
					
					// put to db
#if DATABASE
					//// put this device into database
					//if (DB_IsExist_sensors(dev_host[host].number, dev_host[host].type, UltraSonic_name, 
						//UltraSonic_code, UltraSonic_symbol, 30, 10, UltraSonic_unit) == 0) // not exist
					//{
						//DB_Record_sensors(dev_host[host].number, dev_host[host].type, UltraSonic_name, 
							//UltraSonic_code, UltraSonic_symbol, 30, 10, UltraSonic_unit);
					//}
					//if (DB_IsExist_sensor_types(dev_host[host].type, UltraSonic_name, UltraSonic_description) == 0) // not exist
					//{
						//DB_Record_sensor_types(dev_host[host].type, UltraSonic_name, UltraSonic_description);
					//}
					////DB_Record_sensor_values(dev_host[host].type, UltraSonic_name, UltraSonic_description);
					DB_Record_sensor_values_short(my_float.f, millis()/1000);
#endif

					// adjust time polling

					// save to shared memory

					break;
				case DEV_SENSOR_LIGTH:
					RaspiExt_LED_Hostx_Config(LED_MODE_TOGGLE, 50, host);
					break;
				case DEV_RF:
					break;
				case DEV_BLUETOOTH:
					break;
				case DEV_BUZZER:
					break;
				case DEV_SENSOR_GAS:
					RaspiExt_LED_Hostx_Config(LED_MODE_TOGGLE, 50, host);
					break;
				case DEV_SIM900:
					break;
				case DEV_MY_THESIS:
					RaspiExt_LED_Hostx_Config(LED_MODE_TOGGLE, 1000, host);
					break;
				default:
#if DEVICE_DEBUG
					printf("Thread: %d. host: %d. Unknown device type.\n",
							(int)polling_thread[host], host);
#endif
					dev_host[host].type = DEV_UNKNOWN;
					dev_host[host].number = DEV_NUMBER_UNKNOWN;
					break;
				}
			}
			else // unknown device type
			{
				RaspiExt_LED_Hostx_Config(LED_MODE_OFF, 50, host);
				
#if DEVICE_DEBUG
				printf("Thread: %d. host: %d. Unknown device, identifying.\n",
						(int)polling_thread[host], host);
#endif

				// query broadcast id to identify what it is
				// adjust time polling to 500 ms
				time_poll = 500000;


				trying_time = 0;
				while (pthread_mutex_trylock(&serial_access) != 0)
				{
					usleep(1000);
					trying_time ++;
					if (trying_time > 10)
					{
						break;
					}
				}
				if (trying_time > 10)
				{
#if DEVICE_DEBUG
					printf("Thread: %d. host: %d. Fail to access serial port.\n",
							(int)polling_thread[host], host);
#endif
				}
				else
				{
#if DEVICE_DEBUG
					//dev_host[host].type = DEV_SENSOR_ULTRA_SONIC;
					//dev_host[host].number = 0x01;
#endif
					RaspiExt_Pin_Hostx_Active(host);
					if (queryData(&dev_host[host]))
					{
#if DEVICE_DEBUG
						printf("Thread: %d. host: %d. Got data from device.\n",
								(int)polling_thread[host], host);
#endif
						// TODO: register new id
						unsigned char reg_id = dev_host[host].type | dev_host[host].number;
						if (RegisterID(&reg_id) != 0)
						{
							printf("Thread: %d. host: %d. Fail to register new device.\n",
								(int)polling_thread[host],
								host);
						}
						else
						{
							printf("Thread: %d. host: %d. Registered new device %X.\n",
								(int)polling_thread[host],
								host, 
								reg_id);							
						}
						dev_host[host].type = DEV_TYPE_MASK(reg_id);
						dev_host[host].number = DEV_NUMBER_MASK(reg_id);
						sendControl(dev_host[host]);
					}
					else
					{
#if DEVICE_DEBUG
						printf("Thread: %d. host: %d. No device here.\n",
								(int)polling_thread[host], host);
#endif

					}
					RaspiExt_Pin_Hostx_Inactive(host);

					pthread_mutex_unlock(&serial_access);
				}
			}
			usleep(time_poll);
		}// query device
	}
}
int Device_init(void)
{
	int i = 0;
	printf("Initial Sensor Host.\r\n");
	
	//char *strDeviceName = "Temperature Sensor";	
//
	//int rc = sqlite3_open(dbPath, &db);
//
	//if (rc) {
		//fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		//exit(0);
	//}
	
	pthread_mutex_init(&device_control_access, NULL);
	pthread_mutex_init(&serial_access, NULL);
#if DATABASE
	pthread_mutex_init(&db_token, NULL);
	if (sqlite3_open(dbPath, &db)) 
	{
		printf("Can't open database: %s\n", sqlite3_errmsg(db));
		db = NULL;
		// hold the token
		pthread_mutex_lock(&db_token);
	}
#endif 
	RaspiExt_Init();

	Serial_Init();
	for (i = 0; i < DEV_HOST_NUMBER; i++)
	{
		printf("Initial Sensor Host %d parameters.\r\n", i);
		dev_host[i].data_type = TYPE_FLOAT;
		dev_host[i].number = DEV_NUMBER_UNKNOWN;	// unknown
		dev_host[i].type = DEV_UNKNOWN;		// unknown
		dev_host[i].data = NULL;
		dev_host[i].polling_control.enable = 0;
		dev_host[i].polling_control.time_poll_ms = 200;
		dev_host[i].polling_control.destroy = 0;

		printf("Create thread poll for Sensor Host %d.\r\n", i);
		pthread_create(&polling_thread[i], NULL, &DevicePolling, (void *)i);
	}
	
	// create database push polling
	db_com_init(dev_host);
	return 0;
}
int Device_startPooling(int host_number)
{
	if (host_number < 1 || host_number > 4)
	{
		printf("Invalid Host Number. (1 - 4).\n");
		return -1;
	}
	while(pthread_mutex_trylock(&device_control_access) != 0)
		usleep(100);

	//	pthread_mutex_lock(&device_control_access);
	dev_host[host_number-1].polling_control.enable = 1;
	pthread_mutex_unlock(&device_control_access);

	return 0;
}

int Device_stopPooling(int host_number)
{
	while(pthread_mutex_trylock(&device_control_access) != 0)
		usleep(100);

	//	pthread_mutex_lock(&device_control_access);
	dev_host[host_number].polling_control.enable = 0;
	pthread_mutex_unlock(&device_control_access);

	return 0;
}

int Device_destroyAll(void)
{
	int i;
	while(pthread_mutex_trylock(&device_control_access) != 0)
		usleep(100);

	//	pthread_mutex_lock(&device_control_access);
	for (i = 0; i < DEV_HOST_NUMBER; i++)
	{
		dev_host[i].polling_control.destroy = 1;
	}
	pthread_mutex_unlock(&device_control_access);

	RaspiExt_DestroyAll();

#if DATABASE
	if (db != NULL)
		// Close database
		sqlite3_close(db);
#endif

	return 0;
}

int Device_waitForExit(void)
{
	if (pthread_equal(pthread_self(), polling_thread[0]) == 0)
		pthread_join(polling_thread[0], NULL);
	if (pthread_equal(pthread_self(), polling_thread[1]) == 0)
		pthread_join(polling_thread[1], NULL);
	if (pthread_equal(pthread_self(), polling_thread[2]) == 0)
		pthread_join(polling_thread[2], NULL);
	if (pthread_equal(pthread_self(), polling_thread[3]) == 0)
		pthread_join(polling_thread[3], NULL);
	RaspiExt_WaitForExit();
	return 0;
}
