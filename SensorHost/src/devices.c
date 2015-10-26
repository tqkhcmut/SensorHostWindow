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
#include "wiringPi.h"
#endif


#include <pthread.h>

#define DEV_HOST_NUMBER 4 // 4 USB interfaces

pthread_t polling_thread[DEV_HOST_NUMBER];
struct Device dev_host[DEV_HOST_NUMBER];

// protect device host access
pthread_mutex_t device_control_access = PTHREAD_MUTEX_INITIALIZER;
// protect serial access
pthread_mutex_t serial_access = PTHREAD_MUTEX_INITIALIZER;

int sendControl(struct Device dev)
{
	int packet_len = 3 + getTypeLength(dev.data_type);
	struct Packet * packet = malloc(packet_len);

	packet->id = dev.number + dev.type;
	packet->cmd = CMD_CONTROL;
	packet->data_type = dev.data_type;
	memcpy(packet->data, dev.data, packet_len - 3);

	Serial_SendMultiBytes((unsigned char *) packet, packet_len);
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
		if (checksum(packet) != packet->data[getTypeLength(packet->data_type)])
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

		if (dev->data_type != (packet->data_type & 0x0f))
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
	unsigned char trying_time = 0;
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
			if (dev_host[host].type != 0xff)
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

				switch (dev_host[host].type)
				{
				case DEV_SENSOR_TEMPERATURE:
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
					my_float.f_byte[0] = dev_host[host].data[3];
					my_float.f_byte[1] = dev_host[host].data[2];
					my_float.f_byte[2] = dev_host[host].data[1];
					my_float.f_byte[3] = dev_host[host].data[0];
					printf("Thread: %d. host: %d. Distance: %0.3f.\n",
						(int)polling_thread[host],
						host,
						my_float.f);

					// adjust time polling

					// save to shared memory

					break;
				case DEV_SENSOR_LIGTH:
					break;
				case DEV_RF:
					break;
				case DEV_BLUETOOTH:
					break;
				case DEV_BUZZER:
					break;
				case DEV_SENSOR_GAS:
					break;
				case DEV_SIM900:
					break;
				case DEV_MY_THESIS:
					break;
				default:
#if DEVICE_DEBUG
					printf("Thread: %d. host: %d. Unknown device type.\n",
							(int)polling_thread[host], host);
#endif
					dev_host[host].type = DEV_BROADCAST;
					dev_host[host].number = 0x0f;
					break;
				}
			}
			else
			{
				printf("Thread: %d. host: %d. Unknown device, identifying.\n",
						(int)polling_thread[host], host);

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

	pthread_mutex_init(&device_control_access, NULL);
	pthread_mutex_init(&serial_access, NULL);

	RaspiExt_Init();

	Serial_Init();
	for (i = 0; i < DEV_HOST_NUMBER; i++)
	{
		printf("Initial Sensor Host %d parameters.\r\n", i);
		dev_host[i].data_type = TYPE_FLOAT;
		dev_host[i].number = 0xff;	// unknown
		dev_host[i].type = 0xff;		// unknown
		dev_host[i].data = NULL;
		dev_host[i].polling_control.enable = 0;
		dev_host[i].polling_control.time_poll_ms = 200;
		dev_host[i].polling_control.destroy = 0;

		printf("Create thread poll for Sensor Host %d.\r\n", i);
		pthread_create(&polling_thread[i], NULL, &DevicePolling, (void *)i);
	}
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
