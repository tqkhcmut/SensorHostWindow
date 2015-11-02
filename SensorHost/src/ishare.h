/*
 * ishare.h
 *
 *  Created on: Oct 5, 2015
 *      Author: Tra Quang Kieu
 */

#ifndef SRC_ISHARE_H_
#define SRC_ISHARE_H_

#include "queue.h"
#include <inttypes.h>


// File format
////////////////////////////////////////////////////////////////////////////////////////////
// Sensor Type // Sensor Number // Sensor Data Count // Sensor Data // ... // Sensor Data //
////////////////////////////////////////////////////////////////////////////////////////////

struct iShare
{
	unsigned char SensorType;
	unsigned char SensorNumber;
	Queue_t data_in_queue;
	void * p; // private data, do not modify this
};


struct SharedMemoryData	
{
	unsigned char SensorType;
	unsigned char SensorNumber;
	unsigned int DataCount;
	void * SensorData; // sensor data struct, define in packet.h
};

// use shared memory
#define ISHARE_MEM_SIZE 1048576 // 1Mb for each sensor

// Initial iShare struct, create thread to handle it
// The thread is automatic save data to shared memory, push it to server and save to disk when 
// shared memory is full
extern int iShare_Init(struct iShare * iShare); // return shared memory key
extern int iShare_DeInit(struct iShare * iShare);

// some utility function
extern int iShareGetSHMKey(struct iShare ishare);
extern int iShareGetSavedFilename(struct iShare ishare, char * filename_buffer);
extern int iShare_SaveToDisk(struct SharedMemoryData * ishare_data, const char * filename);
extern int iShare_RestoreFromDisk(struct SharedMemoryData * ishare_data, const char * filename);
#endif /* SRC_ISHARE_H_ */
