/*
 * ishare.h
 *
 *  Created on: Oct 5, 2015
 *      Author: Tra Quang Kieu
 */

#ifndef SRC_ISHARE_H_
#define SRC_ISHARE_H_

#include "queue.h"


// File format
////////////////////////////////////////////////////////////////////////////////////////////
// Sensor Type // Sensor Number // Sensor Data Count // Sensor Data // ... // Sensor Data //
////////////////////////////////////////////////////////////////////////////////////////////

struct iShareHeader
{
	unsigned char SensorType;
	unsigned char SensorNumber;
	Queue_t data_in_queue;
};

/*

// Common mutex protection
pthread_mutex_t iShareAccess = PTHREAD_MUTEX_INITIALIZER;

// Common file name saved on external store (not RAM memory)
const char * iShareFilename = "raspido.dat";

*/

// use shared memory
#define ISHARE_MEM_SIZE 1048576 // 1Mb for each sensor

// Some function
extern int iShare_Init(struct iShareHeader * iShareHeader); // return shared memory key

extern int iShare_SaveToDisk(struct iShareHeader * iShareHeader, const char * filename);
extern int iShare_RestoreFromDisk(struct iShareHeader * iShareHeader, const char * filename);

#endif /* SRC_ISHARE_H_ */
