/*
 * client.c
 *
 *  Created on: Oct 18, 2015
 *      Author: Tra Quang Kieu
 */

#include "client.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int client_fd;


int Client_SendData(struct ClientData data)
{
	if (client_fd > 0)
	{

	}
	return 0;
}
int Client_RecvData(struct ClientData * data)
{

	return 0;
}
int Client_DataAvailable(void)
{

	return 0;
}

int Client_Init(const char * server_address, unsigned int server_port)
{
	struct sockaddr_in serv_addr, cli_addr;

	return 0;
}
