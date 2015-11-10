#include "db_com.h"
#include "src/devices.h"
#include "SQLiteAPI.h"
#include "src/packet.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct insert_data db_push_data;
const char * db_str_format = "%d, %d, %0.3f; %d, %d, %0.3f; %d, %d, %0.3f; %d, %d, %0.3f;";
pthread_t db_com;

void * db_com_thread(void * host_list)
{
	struct Device * hosts = (struct Device *) host_list;
	int db_time_push = 30000; // 30ms
	float_struct_t my_float;
	
	for (;;)
	{
		// collect informations the push t db_push_data
		int i;
		for (i = 0; i < 4; i++) // 4 host
		{
			db_push_data.data[i].host_id = i + 1;
			if (hosts[i].number == DEV_NUMBER_UNKNOWN)
			{
				db_push_data.data[i].sensor_id = 0;
			}
			else
			{
				db_push_data.data[i].sensor_id = hosts[i].number;
			}
			if (hosts[i].type == DEV_UNKNOWN)
			{
				db_push_data.data[i].sensor_type = 0;
			}
			else
			{
				db_push_data.data[i].sensor_type = hosts[i].type;
			}
			
			if (IS_BIG_ENDIAN_BYTE_ORDER(hosts[i].data_type))
			{
				my_float.f_byte[0] = hosts[i].data[3];
				my_float.f_byte[1] = hosts[i].data[2];
				my_float.f_byte[2] = hosts[i].data[1];
				my_float.f_byte[3] = hosts[i].data[0];
			}
			else
			{
				my_float.f_byte[0] = hosts[i].data[0];
				my_float.f_byte[1] = hosts[i].data[1];
				my_float.f_byte[2] = hosts[i].data[2];
				my_float.f_byte[3] = hosts[i].data[3];
			}
			db_push_data.data[i].sensor_value = my_float.f;
		}
		// convert to string
		memset(db_push_data.raw_data, 0, SQL_STR_SIZE);
		sprintf(db_push_data.raw_data, db_str_format, 
			db_push_data.data[0].host_id, db_push_data.data[0].sensor_id, db_push_data.data[0].sensor_value, 
			db_push_data.data[1].host_id, db_push_data.data[1].sensor_id, db_push_data.data[1].sensor_value, 
			db_push_data.data[2].host_id, db_push_data.data[2].sensor_id, db_push_data.data[2].sensor_value,
			db_push_data.data[3].host_id, db_push_data.data[3].sensor_id, db_push_data.data[3].sensor_value);
		// call insert api function
		insert_sensor_value(db_push_data);
		
		// sleep for the next polling
		usleep(db_time_push);
	}
}

void db_com_init(struct Device * host_list)
{
	printf("Create thread poll for database.\r\n");
	pthread_create(&db_com, NULL, &db_com_thread, (void *)host_list);
}