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
	void MySQLDataQuery::create_select_statement(MySQLRelationshipQueryData *res_dat, char *msg, int len) {
		char buff[4096];
		char tmp[128];

		char alias[16], col_alias[16];

		memset(&buff,0,sizeof(buff));
		strcpy(buff, "SELECT ");
		for(int i=0;i<mp_class_desc->num_members;i++) {
			snprintf(alias, sizeof(alias), "mc%d",i);
			sprintf(tmp, "`mt0`.`%s` `%s`,", mp_class_desc->variable_map[i].variable_name, alias);
			strcat(buff, tmp);
		}
		
		if(res_dat && mp_class_desc->relations) {
			for(int i=0;i<mp_class_desc->num_relations;i++) {
				if(mp_class_desc->relations[i].relation_type != ERelationshipType_OneToOne) continue;
				snprintf(alias, sizeof(alias), "mt%d",i+1);
				QueryableClassDesc *target_desc = mp_class_desc->relations[i].target_class_desc;
				for(int j=0;j<target_desc->num_members;j++) {
					snprintf(col_alias, sizeof(col_alias), "mt%d_c%d",i+1,j);
					sprintf(tmp, "`%s`.`%s` `%s`,",alias, target_desc->variable_map[j].variable_name, col_alias);
					strcat(buff, tmp);
				}
			}
		}
		buff[strlen(buff)-1] = 0;
		sprintf(tmp, " FROM `%s` `mt0`",mp_class_desc->tableName);
		strcat(buff, tmp);
		if(res_dat && mp_class_desc->relations) {
			//add join statements
			for(int i=0;i<mp_class_desc->num_relations;i++) {
				QueryableClassRelationshipDesc *relation = &mp_class_desc->relations[i];
				if(relation->relation_type != ERelationshipType_OneToOne) continue;
				snprintf(alias, sizeof(alias), "mt%d",i+1);
				
				sprintf(tmp, " LEFT JOIN `%s` `%s` on `%s`.`%s` = `mt0`.`%s`", relation->target_class_desc->tableName,alias, alias, relation->target_column, relation->source_column);
				strcat(buff, tmp);
			}	
		}
		strcpy(msg, buff);
		printf("rel query: %s\n", buff);
	}
	DataResultSet* MySQLDataQuery::select(QuerySearchParams *search_params, QueryOrder *query_order, QueryLimit *limit, bool with_relations) {
		char query[MYSQL_QUERY_BUFF_SIZE];
		char where[256];
		char order[256];
		char limit_stmt[256];
		where[0] = 0;
		order[0] = 0;
		limit_stmt[0] = 0;
		query[0] = 0;

		MySQLRelationshipQueryData rel_query_data;
		if(with_relations) {
			rel_query_data = get_relationship_data(search_params);
		}
		create_select_statement(with_relations?(MySQLRelationshipQueryData *)&rel_query_data : NULL, (char *)&query, sizeof(query));
		

		if(limit != NULL)
			create_limit_statement(limit, (char *)&limit_stmt, sizeof(limit_stmt));
		//create_order_statement(query_order, (char *)&order, sizeof(order));

		if(search_params) {
			create_where_statement(search_params, where, sizeof(where));
			strcat(query, where);
		}
  	
		if(order[0] != 0)
			strcat(query, order);
	
		if(limit_stmt[0] != 0) 
			strcat(query, limit_stmt);


		printf("Query: %s\n", query);

		mysql_query(mp_data_src->getMySQLConn(), query);
		MYSQL_RES *res = mysql_store_result(mp_data_src->getMySQLConn());

		DataResultSet *result_set = new DataResultSet;
		MYSQL_ROW row;
		while((row = mysql_fetch_row(res))) {
			mysql_field_seek(res, 0);
			result_set->AddObject(create_object_from_row(res, row));
		}
		mysql_free_result(res);
		return result_set;
	}
	MySQLRelationshipQueryData MySQLDataQuery::get_relationship_data(QuerySearchParams *search_params) {
		MySQLRelationshipQueryData ret;
		return ret;
	}
	void *MySQLDataQuery::create_object_from_row(MYSQL_RES *res, MYSQL_ROW row) {
		int pk_row_index = -1, row_pk = 0, related_row_pk_index = -1, related_row_pk = 0;
		DB::QueryVariableMemberMap *pk = getPrimaryKey(mp_class_desc, &pk_row_index);
		DB::QueryVariableMemberMap *related_pk;
		if(pk_row_index != -1) {
			row_pk = atoi(row[pk_row_index]);
		}
		void *obj = mp_class_desc->mpFactoryMethod(mp_data_src, row_pk);
		int num_fields = mysql_num_fields(res);
		MYSQL_FIELD *field;
		sGenericData *data;

		for(int i=0;i<mp_class_desc->num_members;i++) {
			field = mysql_fetch_field(res);
			if(mp_class_desc->variable_map[i].mpSetMethod != NULL) {
				data = getGenericFromString(row[i], mp_class_desc->variable_map[i].dataType);
				mp_class_desc->variable_map[i].mpSetMethod((DB::DataSourceLinkedClass*)obj, data, field->name);
			}
		}

		//load one to one
		int field_offset = mysql_field_tell(res);
		for(int i=0;i<mp_class_desc->num_relations;i++) {
			if(mp_class_desc->relations[i].relation_type != ERelationshipType_OneToOne) continue;
			QueryableClassDesc *target_desc = mp_class_desc->relations[i].target_class_desc;
			related_pk = getPrimaryKey(target_desc, &related_row_pk_index);
			if(related_row_pk_index != -1) {
				related_row_pk = atoi(row[field_offset + related_row_pk_index]);
				//printf("Related row pk: %d\n", related_row_pk);
			}
			void *related_object = target_desc->mpFactoryMethod(mp_data_src,related_row_pk);
			for(int j=0;j<target_desc->num_members;j++) {
				field = mysql_fetch_field(res);
				data = getGenericFromString(row[field_offset + ((i+1)*j)], target_desc->variable_map[j].dataType);
				target_desc->variable_map[j].mpSetMethod((DB::DataSourceLinkedClass *)related_object, data, field->name);
			}
			sGenericData saveData;
			saveData.sUnion.pVoidPtr = related_object;
			saveData.type = EDataType_VoidPtr;
			mp_class_desc->relations[i].mpSetMethod((DB::DataSourceLinkedClass*)obj, &saveData, NULL); 
		}

		//load one to many
		for(int i=0;i<mp_class_desc->num_relations;i++) {
			if(mp_class_desc->relations[i].relation_type != ERelationshipType_OneToMany) continue;
			DB::QueryVariableMemberMap *source_column = getMemberByName(mp_class_desc, mp_class_desc->relations[i].source_column);
			DB::QueryVariableMemberMap *target_column = getMemberByName(mp_class_desc->relations[i].target_class_desc, mp_class_desc->relations[i].target_column);

			sGenericData column_data, pk_data;

			pk_data = source_column->mpGetMethod((DataSourceLinkedClass *)obj, source_column->variable_name);

			DB::ClassColumn related_column;
			related_column.mp_context = mp_class_desc->relations[i].target_class_desc;
			related_column.mp_variable = DB::getMemberByName(mp_class_desc->relations[i].target_class_desc, mp_class_desc->relations[i].target_column);
			related_column.one_to_many = true;

			column_data.type = EDataType_VariableName;
			column_data.sUnion.pVoidPtr = (void *)&related_column;


			DB::QuerySearchParams *where = new DB::QuerySearchParams();
			where->pushOperator(DB::EQueryOperator_Equal);

			where->pushData(&column_data);
			where->pushData(&pk_data);

			MySQLDataQuery *query = new MySQLDataQuery(mp_data_src, mp_class_desc->relations[i].target_class_desc);
			DB::DataResultSet *res = query->select(where);
			Core::Iterator<Core::Vector<void *>, void *> it = res->begin();

			void *insert_obj = *it;
			if(insert_obj) {
				//void (*mpListAppendMethod)(DataSourceLinkedClass *obj, const char *variable_name, DataSourceLinkedClass *child);
				//printf("Calling append: %p\n", mp_class_desc->relations[i].mpListAppendMethod);
				mp_class_desc->relations[i].mpListAppendMethod((DB::DataSourceLinkedClass*)obj, NULL, (DataSourceLinkedClass *)insert_obj);
				//printf("Append called\n");
			}

			delete where;
			delete query;
			delete res;
		}
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

		//TODO: PDO bindings
		
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
	void MySQLDataQuery::create_order_statement(QueryOrder *query_order, char *out, int len) {
		switch(query_order->sort) {
			case EQuerySortMode_Ascending:
				snprintf(out, len, " ORDER BY `%s` ASC",query_order->column->variable_name);
				break;
			case EQuerySortMode_Descending:
				snprintf(out, len, " ORDER BY `%s` DESC",query_order->column->variable_name);
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
		//printf("SQL PTR: %p\n", conn);
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
		int membermap_count = obj->getClassDesc()->num_members;
		DB::QueryVariableMemberMap *memberMap = obj->getClassDesc()->variable_map;
		DB::QueryVariableMemberMap *pk = getPrimaryKey(obj->getClassDesc());
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
		//printf("Output query: %s\n", query);
	}
	DB::DataRow *MySQLDataSource::repullObj(DB::DataSourceLinkedClass *obj) {

	}
	void MySQLDataSource::getMySQLPrintFmt(DB::DataSourceLinkedClass *obj, DB::QueryVariableMemberMap *map, char *msg, int len) {

		sGenericData data = (map->mpGetMethod(obj, map->variable_name));
		getMySQLPrintFmt((sGenericData *)&data, msg, len);

	}
	void MySQLDataSource::getMySQLVariableFmt(sGenericData *data, char *out, int size) {
		ClassColumn *col = (ClassColumn *)data->sUnion.pVoidPtr;
		int tbl_idx = getTableOffset(col->mp_context, col->mp_variable, col->one_to_many);

		snprintf(out, size, "`mt%d`.`%s`",tbl_idx, col->mp_variable->variable_name);

		printf("Tbl idx: %d (%s)\n",tbl_idx, out);
	}
	void MySQLDataSource::getMySQLPrintFmt(sGenericData *data, char *out, int size) {
		char vardata[256];
		char identifier = 0;
		switch(data->type) {
			case EDataType_String_ASCII:
			identifier = '\"';
			break;
			case EDataType_VariableName:
			//identifier = '`';
			getMySQLVariableFmt(data,out,size);
			return;
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