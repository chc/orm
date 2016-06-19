#ifndef _MYSQLDS_H
#define _MYSQLDS_H
#include <mysql.h>
#include "DataSource.h"
namespace DB {
	class MySQLDataSource : public DataSource {
		public:
			MySQLDataSource();
			~MySQLDataSource();
			void connect(const char *username, const char *password, const char *server, const char *database);
			void disconnect();
			DataQuery* makeSelectQuery(QueryableClassDesc *class_desc, QuerySearchParams *params);
			MYSQL *getMySQLConn() { return conn; };
		private:
			MYSQL *conn;
	};
	class MySQLDataQuery : public DataQuery {
		public:
			MySQLDataQuery(MySQLDataSource *source, QueryableClassDesc *class_desc);
			DataRow* select(int pk_id);
			DataResultSet* select(QuerySearchParams *search_params);
			DataRow* remove(int pk_id);
			DataResultSet* remove(QuerySearchParams *search_params);
		private:
			void build_base_query();
			char *mp_base_query;
			MySQLDataSource *mp_data_src;
	};

}
#endif //_MYSQLDS_H