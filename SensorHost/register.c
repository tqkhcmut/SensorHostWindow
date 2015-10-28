#include "register.h"

#include "packet.h"
#include <pthread.h>

struct RegData
{
	unsigned char type;
	unsigned char number[14];
	struct RegData * next;
};

pthread_mutex_t reg_token = PTHREAD_MUTEX_INITIALIZER;

struct RegData * registered = NULL;

int ValidationID(unsigned char * id)
{
	struct RegData * tmpReg;
	if (IS_BROADCAST_ID(*id))
		return -1;
	pthread_mutex_lock(&reg_token);
	tmpReg = registerde;
	while (tmpReg != NULL)
	{
		if (tmpReg->type == DEV_TYPE_MASK(*id))
		{
			unsigned char i, tmpNum;
			tmpNum = DEV_NUMBER_MASK(*id);
			if (tmpNum == 0 || tmpNum > 14)
			{
				for (i = 0; i < 14; i++)
				{
					if (tmpReg->number[i] == 0)
					{
						*id = DEV_TYPE_MASK(tmpReg->type) | DEV_NUMBER_MASK(i + 1);
						tmpReg->number[i] = DEV_NUMBER_MASK(*id);
					}
				}
			}
			else
			{
check_again:
				for (i = 0; i < 14; i++)
				{
					if (tmpReg->number[i] == 0)
					{
						
					}
					if (tmpReg->number[i] == tmpNum)
					{
						// this mean this id have been registed
						tmpNum++;
						goto check_again;
					}
				}
				pthread_mutex_unlock(&reg_token);
				return -1;
			}
		}
	}
	pthread_mutex_unlock(&reg_token);
	return RegisterID(*id);
}
int RegisterID(unsigned char id)
{
	
}
int UnRegisterID(unsigned char id)
{
	
}
