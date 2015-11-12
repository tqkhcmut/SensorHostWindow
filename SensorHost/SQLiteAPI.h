#ifndef SQLiteAPI_
#define SQLiteAPI_

#ifdef __cplusplus
extern "C" {
#endif

#define SQL_STR_SIZE 512 // I think this is large enough to store string

struct sensor_data
{
	unsigned char sensor_type;
	unsigned char host_id;
	unsigned char sensor_id;
	float sensor_value;
};

struct insert_data
{
	struct sensor_data data[4];
	/*
		 char *raw_data= "host_id_1,sensor_id_1,sensor_value_1; host_id_2, sensor_id_2, sensor_value_2;
			host_id_3, sensor_id_3, sensor_value_3;
			host_id_4, sensor_id_4, sensor_value_4";
	*/
	char raw_data[SQL_STR_SIZE]; 
};

	int ConnectDB(void);
	void DisonnectDB(void);

	int insert_sensor_value(struct insert_data data);
	int is_collect_data(void);

#ifdef __cplusplus
}
#endif
#endif
