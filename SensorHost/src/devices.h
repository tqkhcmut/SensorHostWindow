/*
 * sensor.h
 *
 *  Created on: Sep 17, 2015
 *      Author: kieu
 */

#ifndef DEVICES_H_
#define DEVICES_H_

//#include "global_config.h"
#include "packet.h"
#include <pthread.h>


#ifndef DEVICE_DEBUG
#define DEVICE_DEBUG 0
#endif

#ifndef DATABASE
#define DATABASE 0
#endif


struct polling_control
{
	unsigned char destroy;
	unsigned char enable;
	unsigned int time_poll_ms;
};

struct Device
{
	unsigned char number;
	unsigned char type;

	struct polling_control polling_control;

	unsigned char data_type;
	unsigned char * data;
};

struct HostInfo
{
	unsigned char buzzer_available;
	unsigned char sim900_available;
	unsigned char rf_available;
};

union float_struct
{
	float f;
	unsigned char f_byte[4];
};
typedef union float_struct float_struct_t;

extern int Device_init(void);
extern int Device_startPooling(int host_number);
extern int Device_stopPooling(int host_number);
extern int Device_destroyAll(void);
extern int Device_waitForExit(void);


extern int sendControl(struct Device dev);
extern int queryData(struct Device * dev);

extern pthread_mutex_t device_control_access;

#endif /* SENSOR_H_ */
