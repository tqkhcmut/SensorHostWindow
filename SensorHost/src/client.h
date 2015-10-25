/*
 * client.h
 *
 *  Created on: Oct 18, 2015
 *      Author: Tra Quang Kieu
 */

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_

struct ClientData
{
	unsigned long time;
	unsigned int warning_level;
	float Temperature;
	float LightingLevel;
	float Gas;
};

int Client_SendData(struct ClientData data);
int Client_RecvData(struct ClientData * data);
int Client_DataAvailable(void);

int Client_Init(const char * server_address, unsigned int server_port);

#endif /* SRC_CLIENT_H_ */
