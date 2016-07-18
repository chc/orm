#include "DataSource.h"

namespace DB {
	DataSource::DataSource() {

	}
	DataSource::~DataSource() {

	}
	DataQuery::DataQuery(QueryableClassDesc *class_desc) {
		mp_class_desc = class_desc;	
	}
	void DataSourceLinkedClass::remove() {
		mp_data_src->removeObj(this);
	}
	void DataSourceLinkedClass::save() {
		mp_data_src->saveObj(this);
	}
	void DataSourceLinkedClass::repull() {

	}
	void DataSourceLinkedClass::lock() {

	}
	void DataSourceLinkedClass::unlock() {

	}
/*
	sGenericData getDataFromMemberMap(DB::QueryVariableMemberMap *map) {
		return map->mpGetMethod(this, map->variable_name);
	}
*/
	QuerySearchParams::QuerySearchParams() {

	}
	QuerySearchParams::~QuerySearchParams() {

	}
	void QuerySearchParams::pushOperator(EQueryOperator op) {
		m_query_operators.add(op);
	}
	void QuerySearchParams::pushData(sGenericData *data) {
		m_query_data.add(data);
	}

	DB::QueryVariableMemberMap *getPrimaryKey(QueryableClassDesc *class_desc, int *output_index) {
		return getMemberByName(class_desc,"id", output_index);
	}
	
	DB::QueryVariableMemberMap *getMemberByName(QueryableClassDesc *class_desc, const char *name, int *output_index) {
		for(int i=0;i<class_desc->num_members;i++) {
			if(strcmp(name, class_desc->variable_map[i].variable_name) == 0) {
				if(output_index) {
					*output_index = i;
				}
				return &class_desc->variable_map[i];
			}
		}
		return NULL;
	}
	int getTableOffset(QueryableClassDesc *class_desc, DB::QueryVariableMemberMap *memberMap, bool one_to_many) {
		int num_members;
		QueryVariableMemberMap *variable_map;

		for(int i=0;i<class_desc->num_members;i++) {
			if(memberMap == (DB::QueryVariableMemberMap *)&class_desc->variable_map[i]) {
				return 0; //source table
			}
		}
		for(int i=0;i<class_desc->num_relations;i++) {
			QueryableClassDesc *desc = class_desc->relations[i].target_class_desc;
			if(!one_to_many && class_desc->relations[i].relation_type != ERelationshipType_OneToOne)  {
				continue;
			}
			if(one_to_many && class_desc->relations[i].relation_type != ERelationshipType_OneToMany) {
				continue;
			}
			for(int j=0;j<desc->num_members;j++) {
				if(((DB::QueryVariableMemberMap *)&desc->variable_map[j]) == memberMap) {
					return i+1;
				}
			}
		}
		return -1;
	}
	int getVariableMapOffset(QueryableClassDesc *class_desc, DB::QueryVariableMemberMap *memberMap) {
		return -1;
	}
}