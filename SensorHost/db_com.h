#ifndef _db_com_h_
#define _db_com_h_

#ifndef DB_COM_DEBUG
#define DB_COM_DEBUG 1
#endif
#include "src/devices.h"

void db_com_init(struct Device * host_list);

#endif // !_db_com_h_
