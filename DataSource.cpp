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
	DB::QueryVariableMemberMap *DataSourceLinkedClass::getPrimaryKey() {
		DB::QueryVariableMemberMap *memberMap;
		int num_members;
		memberMap = this->getMemberMap(num_members);
		DB::QueryVariableMemberMap *pk = getMemberByName("id", memberMap, num_members);
		return pk;
	}
	
	DB::QueryVariableMemberMap *getMemberByName(const char *name, DB::QueryVariableMemberMap *memberMap, int num_instances) {
		for(int i=0;i<num_instances;i++) {
			if(strcmp(name, memberMap[i].variable_name) == 0) {
				return &memberMap[i];
			}
		}
		return NULL;
	}
	sGenericData DataSourceLinkedClass::getDataFromMemberMap(DB::QueryVariableMemberMap *map) {
		return map->mpGetMethod(this, map->variable_name);
	}
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


}