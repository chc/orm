#include "MySQLDataSource.h"
namespace DB {

///////////////////////////////////////////
////// MySQL Query implementation
	MySQLDataQuery::MySQLDataQuery(MySQLDataSource *source, QueryableClassDesc *class_desc) : DataQuery(class_desc) {
		mp_data_src = source;
		build_base_select_query();
	}
	DataRow* MySQLDataQuery::select(int pk_id) {
		return NULL;
	}
	DataResultSet* MySQLDataQuery::select(QuerySearchParams *search_params) {
		mysql_query(mp_data_src->getMySQLConn(), mp_base_select_query);
		MYSQL_RES *res = mysql_store_result(mp_data_src->getMySQLConn());

		DataResultSet *result_set = new DataResultSet;
		MYSQL_ROW row;
		int num_fields = mysql_num_fields(res);
		while((row = mysql_fetch_row(res))) {
			mysql_field_seek(res, 0);
			result_set->AddObject(create_object_from_row(res, row));
		}
		mysql_free_result(res);
		return result_set;
	}
	void *MySQLDataQuery::create_object_from_row(MYSQL_RES *res, MYSQL_ROW row) {
		void *obj = mp_class_desc->mpFactoryMethod(mp_data_src);
		for(int i=0;i<mp_class_desc->num_members;i++) {
			MYSQL_FIELD *field = mysql_fetch_field(res);
			if(mp_class_desc->variable_map[i].mpSetMethod != NULL) {
				sGenericData *data;
				data = getGenericFromString(row[i], mp_class_desc->variable_map[i].dataType);
				mp_class_desc->variable_map[i].mpSetMethod((DB::DataSourceLinkedClass*)obj, data, field->name);
			}
		}
		printf("Ret factory: %p\n", obj);
		return obj;
	}
	DataRow* MySQLDataQuery::remove(int pk_id) {
		return NULL;
	}
	DataResultSet* MySQLDataQuery::remove(QuerySearchParams *search_params) {
		return NULL;
	}
	void MySQLDataQuery::build_base_select_query() {
		/*
			TODO: make string class to build large queries
		*/
		char buff[4096];
		char tmp[128];
		memset(&buff,0,sizeof(buff));
		strcpy(buff, "SELECT ");
		for(int i=0;i<mp_class_desc->num_members;i++) {
			sprintf(tmp, "`%s`,", mp_class_desc->variable_map[i].variable_name);
			strcat(buff, tmp);
		}
		buff[strlen(buff)-1] = 0;
		sprintf(tmp, " FROM `%s`",mp_class_desc->tableName);
		strcat(buff, tmp);


		mp_base_select_query = strdup(buff);
	}

///////////////////////////////////////////
///// MySQL Data Source implementation
	MySQLDataSource::MySQLDataSource() {
		conn = mysql_init(NULL);
		printf("SQL PTR: %p\n", conn);
	}
	MySQLDataSource::~MySQLDataSource() {
		mysql_close(conn);
	}
	void MySQLDataSource::connect(const char *username, const char *password, const char *server, const char *database) {
		if(mysql_real_connect(conn, server, username, password, database, 0, NULL, 0)) {
			fprintf(stderr, "%s\n", mysql_error(conn));
		} else {
			fprintf(stderr, "Succesful mysql connection\n");
		}

	}
	void MySQLDataSource::disconnect() {

	}
	DataQuery* MySQLDataSource::makeSelectQuery(DB::QueryableClassDesc *class_desc, QuerySearchParams *params) {

		DataQuery *ret = new MySQLDataQuery(this, class_desc);

		return ret;
	}
	void MySQLDataSource::removeObj(DB::DataSourceLinkedClass *obj) {

	}
	void MySQLDataSource::saveObj(DB::DataSourceLinkedClass *obj) {
		int membermap_count;
		DB::QueryVariableMemberMap *memberMap = obj->getMemberMap(membermap_count);
		for(int i=0;i<membermap_count;i++) {
			printf("Save: %s\n", memberMap[i].variable_name);
		}
	}
	DB::DataRow *MySQLDataSource::repullObj(DB::DataSourceLinkedClass *obj) {

	}
}