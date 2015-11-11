#include "register.h"

#include "src/packet.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct RegData
{
	unsigned char type;
	unsigned short number_mask;
	struct RegData * next;
};

pthread_mutex_t reg_token = PTHREAD_MUTEX_INITIALIZER;

struct RegData * registered = NULL;

int RegisterID(unsigned char * id)
{
	struct RegData * tmpReg;
	if (IS_BROADCAST_ID(*id))
		return -1;
	pthread_mutex_lock(&reg_token);
	tmpReg = registered;
	while (tmpReg != NULL)
	{
		if (tmpReg->type == DEV_TYPE_MASK(*id))
		{
			unsigned char i, tmpNum;
			tmpNum = DEV_NUMBER_MASK(*id);
			// scan for new number
			for (i = 0; i < 16; i++)
			{
				if (((1 << i) & tmpReg->number_mask) == 0)
				{
					// got it
					tmpNum = i + 1;
					tmpReg->number_mask |= (1 << i);
					*id = tmpReg->type | tmpNum;
					pthread_mutex_unlock(&reg_token);
					return 0;	
				}
			}
			// cannot found empty position
			pthread_mutex_unlock(&reg_token);
			return -1;
		}
		tmpReg = tmpReg->next;
	}
	// device not in register list, create it
	tmpReg = malloc(sizeof(struct RegData));
	tmpReg->type = DEV_TYPE_MASK(*id);
	tmpReg->number_mask = 0x0001;
	tmpReg->next = registered;
	registered = tmpReg;
	pthread_mutex_unlock(&reg_token);
	return 0;
}
int UnRegisterID(unsigned char * id)
{
	struct RegData * tmpReg;
	if (IS_BROADCAST_ID(*id))
		return -1;
	pthread_mutex_lock(&reg_token);
	tmpReg = registered;
	while (tmpReg != NULL)
	{
		if (tmpReg->type == DEV_TYPE_MASK(*id))
		{
			unsigned char i, tmpNum;
			tmpNum = DEV_NUMBER_MASK(*id);
			tmpReg->number_mask &= ~(1 << (tmpNum - 1));
		}
		tmpReg = tmpReg->next;
	}
	pthread_mutex_unlock(&reg_token);
	return 0;
}
