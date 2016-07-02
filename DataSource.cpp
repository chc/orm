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
	
	DB::QueryVariableMemberMap *getMemberByName(const char *name, DB::QueryVariableMemberMap *memberMap, int num_instances) {
	for(int i=0;i<num_instances;i++) {
		if(strcmp(name, memberMap[i].variable_name) == 0) {
			return &memberMap[i];
		}
	}
	return NULL;
}

}