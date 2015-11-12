#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SQLiteAPI.h"

#define DB "PTFEDB.s3db"

int isOpenDB = 0;
sqlite3 *dbfile;
int ConnectDB(void)
{
	if (sqlite3_open(DB, &dbfile) == SQLITE_OK)
	{
		isOpenDB = 1;
		return 1;
	}

	return 0;
}

void DisonnectDB(void)
{
	if (isOpenDB == 1)
	{
		sqlite3_close(dbfile);
	}
}

int insert_sensor_value(struct insert_data data)
{
	sqlite3_stmt *statement;
	int result = -1;	
	char query[1024] = "INSERT INTO sensor_values(sensor_values_id,timestamp, raw_value) VALUES(NULL,0.030, '";
	char end_query[] = "')";

	if (!is_collect_data())
	{
		return -1;
	}

	strcat(query, data.raw_data);
	strcat(query, end_query);	
	{
		if (ConnectDB())
		{			
			int prepare_query = sqlite3_prepare(dbfile, query, -1, &statement, 0);
			if (prepare_query == SQLITE_OK)
			{
				int res = sqlite3_step(statement);
				result = res;
				sqlite3_finalize(statement);
			}
			DisonnectDB();
			return result;
		}
		else
		{
			result = 0;
		}
	}
	return result;
}

int is_collect_data(void)
{
	int result = 0;
	sqlite3_stmt *statement;

	char *query = "select * from settings where setting_key = 'IS_COLLECT_DATA'";
	if (ConnectDB())
	{
		if (sqlite3_prepare(dbfile, query, -1, &statement, 0) == SQLITE_OK)
		{
			int ctotal = sqlite3_column_count(statement);
			int res = 0;

			while (1)
			{
				res = sqlite3_step(statement);

				if (res == SQLITE_ROW)
				{
					/*for (int i = 0; i < ctotal; i++)
					{
						char* s = (char*)sqlite3_column_text(statement, i);
						if (s == "IS")
						{

						}
						printf(s);
					}*/									
					char* tempSettingKey = (char*)sqlite3_column_text(statement, 2);
					if (strcmp(tempSettingKey,"1") == 0)
					{
						sqlite3_finalize(statement);
						DisonnectDB();
						result = 1;
						return 1;
					}
					else
					{
						sqlite3_finalize(statement);
						DisonnectDB();
						return 0;
					}

				}

				if (res == SQLITE_DONE)
				{
					printf("done");
					break;
				}

			}
		}
	}	
	DisonnectDB();
	return result;
}
