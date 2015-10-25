/*
 * main.c
 *
 *  Created on: Sep 22, 2015
 *      Author: kieutq
 */

#include <stdio.h>
#include <stdlib.h>

#include "devices.h"

#ifdef __linux
#include <unistd.h>
#include <signal.h>
#endif

void die(int err)
{
	Device_destroyAll();
	printf("Program exit with code: %d\n", err);
	exit(err);
}


int main (int argc, char * argv[])
{
	signal(SIGINT, die);
	printf("Sensor Host.\n");

	Device_init();

	Device_startPooling(1);
	Device_startPooling(2);
	Device_startPooling(3);
	Device_startPooling(4);


	printf("Sensor Host: enter sleep for other threads work.\n");

	Device_waitForExit();

	system("shutdown -h now");

	return 0;
}
