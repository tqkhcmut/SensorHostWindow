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
#include <pthread.h>

struct iSharePrivate
{
	unsigned char destroy;
	pthread_mutex_t token;
	char * shm_pointer;
};

void * DevicePolling(void * handle_obj); // thread

int iShare_Init(struct iShare * iShare)
{
	key_t key; /* key to be passed to shmget() */
	int shmflg; /* shmflg to be passed to shmget() */
	int shmid; /* return value from shmget() */
	int size; /* size to be passed to shmget() */
	char * shm;
	struct iSharePrivate * private_struct;

	key = iShareGetSHMKey(*iShare);
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

	private_struct = (struct iSharePrivate *)iShare->p;
	private_struct->destroy = 0;
	pthread_mutex_init(&private_struct->token, NULL);
	private_struct->shm_pointer = shm;

	// create thread to handle this iShare
	
	return (int) key;
}

int iShare_DeInit(struct iShare * iShare)
{
	// unsafe code
	struct iSharePrivate * pri = (struct iSharePrivate *)iShare->p;
	pthread_mutex_t * mutex = &pri->token;
	// unsafe code
	// just send destroy command to thread, all RAM data will be free by thread handle it
	pthread_mutex_lock(mutex);
	pri->destroy = 1;
	pthread_mutex_unlock(mutex);
	return 0;
}


// some utility function
int iShareGetSHMKey(struct iShare ishare)
{
	return ishare.SensorType * 100 + ishare.SensorNumber;
}
int iShareGetSavedFilename(struct iShare ishare, char * filename_buffer)
{
	char filename[30];
	memset(filename, 0, 30);
	sprintf(filename, "RaspiDo_%d_%d.tqk", ishare.SensorType, ishare.SensorNumber);
	strcpy(filename_buffer, filename);
	return 0;
}


int iShare_SaveToDisk(struct SharedMemoryData * ishare_data, const char * filename)
{
	struct iSharePrivate * pri;
	
	FILE * fp = fopen(filename, "w+");
	if (fp == NULL)
	{
		printf("Cannot open file: %s.\n", filename);
		return -1;
	}
	
	// check file empty or not
	// if empty, append header then copy data
	// if not, modify DataCount in header then append data at the end of file

	fclose(fp);

	return 0;
}

int iShare_RestoreFromDisk(struct SharedMemoryData * ishare_data, const char * filename)
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

void * DevicePolling(void * handle_obj)
{
	

	pthread_exit(NULL);
}
