#ifndef _MYSQLDS_H
#define _MYSQLDS_H
#include <mysql.h>
#include "DataSource.h"
#define MYSQL_QUERY_BUFF_SIZE 4096
namespace DB {
	class MySQLDataSource : public DataSource {
		public:
			MySQLDataSource();
			~MySQLDataSource();
			void connect(const char *username, const char *password, const char *server, const char *database);
			void disconnect();
			DataQuery* makeSelectQuery(QueryableClassDesc *class_desc, QuerySearchParams *params);
			MYSQL *getMySQLConn() { return conn; };
			void removeObj(DB::DataSourceLinkedClass *obj);
			void saveObj(DB::DataSourceLinkedClass *obj);
			DB::DataRow *repullObj(DB::DataSourceLinkedClass *obj);
		  static void getMySQLPrintFmt(DB::DataSourceLinkedClass *obj, DB::QueryVariableMemberMap *map, char *out, int size);
 		  static void getMySQLPrintFmt(sGenericData *data, char *out, int size);
		private:
			MYSQL *conn;
	};
	typedef struct {

	} MySQLRelationshipQueryData;
	class MySQLDataQuery : public DataQuery {
		public:
			MySQLDataQuery(MySQLDataSource *source, QueryableClassDesc *class_desc);
			DataRow* select(int pk_id);
			DataResultSet* select(QuerySearchParams *search_params, QueryOrder *query_order, QueryLimit *limit, bool with_relationships);
			DataRow* remove(int pk_id);
			DataResultSet* remove(QuerySearchParams *search_params);
			void *create_object_from_row(MYSQL_RES *res, MYSQL_ROW row);
		private:
			MySQLRelationshipQueryData get_relationship_data(QuerySearchParams *search_params);
			void create_select_statement(MySQLRelationshipQueryData *res_dat, char *msg, int len);
			void build_base_select_query();
			void create_where_statement(QuerySearchParams *search_params, char *out, int len);
	        void create_limit_statement(QueryLimit *limit, char *out, int len);
 		    void create_order_statement(QueryOrder *query_order, char *out, int len);
			char *mp_base_select_query;
			MySQLDataSource *mp_data_src;
	};

}
#endif //_MYSQLDS_H