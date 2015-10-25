/*
 * sensor.h
 *
 *  Created on: Sep 17, 2015
 *      Author: kieu
 */

#ifndef DEVICES_H_
#define DEVICES_H_


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


extern int Device_init(void);
extern int Device_startPooling(int host_number);
extern int Device_stopPooling(int host_number);
extern int Device_destroyAll(void);
extern int Device_waitForExit(void);


extern int sendControl(struct Device dev);
extern int queryData(struct Device * dev);

#endif /* SENSOR_H_ */
