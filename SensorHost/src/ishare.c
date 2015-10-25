/*
 * ishare.c
 *
 *  Created on: Oct 5, 2015
 *      Author: Tra Quang Kieu
 */

#include "ishare.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <string.h>


int iShare_Init(struct iShareHeader * iShareHeader)
{
	key_t key; /* key to be passed to shmget() */
	int shmflg; /* shmflg to be passed to shmget() */
	int shmid; /* return value from shmget() */
	int size; /* size to be passed to shmget() */
	char * shm;

	key = iShareHeader->SensorType * 100 + iShareHeader->SensorNumber;
	shmflg = IPC_CREAT | 0666;
	size = ISHARE_MEM_SIZE;

	/*
	 * Create the segment.
	 */
	if ((shmid = shmget(key, size, shmflg)) < 0) {
		printf("shmget error.\n");
		return -1;
	}

	/*
	 * Now we attach the segment to our data space.
	 */
	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
		printf("shmat error");
		return -1;
	}

	memcpy(shm, iShareHeader, sizeof(struct iShareHeader));

	return (int) key;
}

int iShare_SaveToDisk(struct iShareHeader * iShareHeader, const char * filename)
{
	FILE * fp = fopen(filename, "w+");
	if (fp == NULL)
	{
		printf("Cannot open file: %s.\n", filename);
		return -1;
	}
	fwrite((const char *)&(iShareHeader->SensorType), sizeof(unsigned char), 1, fp);
	fwrite((const char *)&(iShareHeader->SensorNumber), sizeof(unsigned char), 1, fp);
	fwrite((const char *)&(iShareHeader->SensorDataCount), sizeof(unsigned int), 1, fp);

	struct iShareData * tmpData = iShareHeader->SensorData;
	while(tmpData != NULL)
	{
		fwrite((const char *)&(tmpData->time), sizeof(float), 1, fp);
		fwrite((const char *)&(tmpData->data), sizeof(float), 1, fp);
		tmpData = tmpData->next;
	}

	fclose(fp);

	return 0;
}

int iShare_RestoreFromDisk(struct iShareHeader * iShareHeader, const char * filename)
{
	FILE * fp = fopen(filename, "r+");
	if (fp == NULL)
	{
		printf("Cannot open file: %s.\n", filename);
		return -1;
	}

	fclose(fp);

	return 0;
}
