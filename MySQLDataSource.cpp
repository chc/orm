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
	DataResultSet* MySQLDataQuery::select(QuerySearchParams *search_params, EQuerySortMode sort_mode, QueryLimit *limit) {
		char query[MYSQL_QUERY_BUFF_SIZE];
		char where[256];
		char order[256];
		char limit_stmt[256];
		where[0] = 0;
		order[0] = 0;
		limit_stmt[0] = 0;
		

		create_where_statement(search_params, where, sizeof(where));
		create_limit_statement(limit, (char *)&limit_stmt, sizeof(limit_stmt));
		create_order_statement(sort_mode, (char *)&order, sizeof(order));
		strcpy(query, mp_base_select_query);
		strcat(query, where);
  	
		if(order[0] != 0)
			strcat(query, order);
	
		if(limit_stmt[0] != 0) 
			strcat(query, limit_stmt);


		printf("Query: %s\n", query);

		mysql_query(mp_data_src->getMySQLConn(), query);
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
		char where[1024];
		create_where_statement(search_params, where, sizeof(where));
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
	void MySQLDataQuery::create_where_statement(QuerySearchParams *search_params, char *out, int len) {
		char where[1024];
		char temp[128];
		where[0] = 0;

		char temp_vardata[2][256];
		strcat(where, " WHERE ");

		int operator_count = 0;
		const char *op = NULL;

		Core::Iterator<Core::Vector<sGenericData *>, sGenericData *> it = search_params->dataBegin();
		sGenericData *data[2];
		Core::Iterator<Core::Vector<EQueryOperator>, EQueryOperator> it2 = search_params->opsBegin();
		while(it2 != search_params->opsEnd()) {
			EQueryOperator type = *it2;
			switch(type) {
				case EQueryOperator_Equal: {
					operator_count = 2;
					op = "=";
					break;
				}
				case EQueryOperator_NotEqual: {
					operator_count = 2;
					op = "!=";
					break;
				}
				case EQueryOperator_Greater: {
					operator_count = 2;
					op = ">";
					break;
				}
				case EQueryOperator_GreaterEqual: {
					operator_count = 2;
					op = ">=";
					break;
				}
				case EQueryOperator_Less: {
					operator_count = 2;
					op = "<" ;
					break;
				}
				case EQueryOperator_LessEqual: {
					operator_count = 2;
					op = "<=";
					break;
				}
				case EQueryOperator_And: {
					operator_count = 0;
					op = "AND";
					break;
				}
				case EQueryOperator_Or: {
					operator_count = 0;
					op = "OR";
					break;
				}
				case EQueryOperator_Not: {
					operator_count = 0;
					op = "NOT";
					break;
				}
			}
			if(operator_count == 2) {
				data[0] = *(it);
				it++;
				data[1] = *(it);
				it++;
				MySQLDataSource::getMySQLPrintFmt(data[0],(char *)&temp_vardata[0], sizeof(temp_vardata[0]));
				MySQLDataSource::getMySQLPrintFmt(data[1],(char *)&temp_vardata[1], sizeof(temp_vardata[1]));
				sprintf(temp, "%s %s %s", temp_vardata[0], op, temp_vardata[1]);
				strcat(where, temp);
			
			} else if(operator_count == 0) {
				sprintf(temp, " %s ", op);
				strcat(where, temp);
			}
			it2++;
		}
		snprintf(out, len, where);
	}
	void MySQLDataQuery::create_order_statement(EQuerySortMode sort, char *out, int len) {
		switch(sort) {
			case EQuerySortMode_Ascending:
				strcat(out, " ORDER BY ASC");
				break;
			case EQuerySortMode_Descending:
				strcat(out, " ORDER BY DESC");
				break;

		}
	}
	void MySQLDataQuery::create_limit_statement(QueryLimit *limit, char *out, int len) {
		snprintf(out, len, " LIMIT %d,%d",limit->offset, limit->row_count);
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
		DB::QueryVariableMemberMap *pk = obj->getPrimaryKey();
		char query[1024];
		char temp[512];
		char temp_vardata[256];
		sprintf(query, "UPDATE `%s` SET ", obj->getClassDesc()->tableName);
		for(int i=0;i<membermap_count;i++) {
			if(&memberMap[i] != pk) {
				getMySQLPrintFmt(obj, &memberMap[i], (char *)&temp_vardata, sizeof(temp_vardata));
				sprintf(temp, "`%s` = %s,", memberMap[i].variable_name, temp_vardata);
				strcat(query, temp);
			}
		}
		query[strlen(query)-1] = 0;
		sGenericData pk_data = pk->mpGetMethod(obj, pk->variable_name);
    	MySQLDataSource::getMySQLPrintFmt(obj, pk, temp_vardata, sizeof(temp_vardata));
		snprintf(temp, sizeof(temp), " where `%s` = %s\n", pk->variable_name, temp_vardata);
		strcat(query, temp);
		printf("Output query: %s\n", query);
	}
	DB::DataRow *MySQLDataSource::repullObj(DB::DataSourceLinkedClass *obj) {

	}
	void MySQLDataSource::getMySQLPrintFmt(DB::DataSourceLinkedClass *obj, DB::QueryVariableMemberMap *map, char *msg, int len) {

		sGenericData data = (map->mpGetMethod(obj, map->variable_name));
		getMySQLPrintFmt((sGenericData *)&data, msg, len);

	}
	void MySQLDataSource::getMySQLPrintFmt(sGenericData *data, char *out, int size) {
		char vardata[256];
		char identifier = 0;
		switch(data->type) {
			case EDataType_String_ASCII:
			identifier = '\"';
			break;
			case EDataType_VariableName:
			identifier = '`';
			break;
		}
		getGenericAsString(data, (char *)&vardata, sizeof(vardata));	
		if(identifier == 0) {
			snprintf(out, size, "%s",vardata);
		} else {
			snprintf(out, size, "%c%s%c",identifier,vardata,identifier);
		}
	}
}