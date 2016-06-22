#ifndef _DATASOURCE_H
#define _DATASOURCE_H
#include "DataBank.h"
#include "Vector.h"
#include "Iterator.h"


namespace DB {
	class DataQuery;
	class DataSourceLinkedClass;

	class DataRow {
		public:
			DataRow();
			const char *getName() const;
			const sGenericData *getData() const;
	};

	class DataResultSet {
		public:
			DataResultSet() { };
			void AddObject(void *obj) { m_object_list.add(obj); };
			Core::Iterator<Core::Vector<void *>, void *> begin() {  return m_object_list.begin();};
			Core::Iterator<Core::Vector<void *>, void *> end() { return m_object_list.end(); };
			int row_count() const;
			const DataRow *get_row(int index) const;
		private:
			Core::Vector<void *> m_object_list;

	};


	////////////////////////
	/// Query Search
	enum EQueryOperator {
		EQueryOperator_Greater,
		EQueryOperator_Less,
		EQueryOperator_GreaterThan,
		EQueryOperator_LessThan,
		EQueryOperator_NotEqual,
		EQueryOperator_Equal,
		EQueryOperator_Like,
	};
	class QuerySearchParams {
		public:
			QuerySearchParams();
			~QuerySearchParams();
			void pushOperator(EQueryOperator op);
			void pushData(sGenericData *data);
		private:
			Core::Vector<sGenericData *> m_query_data;
			Core::Vector<EQueryOperator> m_query_operators;
	};


	////////////////////////////
	//// Query class descriptor
	typedef struct {
		const char *variable_name;
		EDataType dataType;
		void (*mpSetMethod)(DataSourceLinkedClass *obj, sGenericData *data, const char *variable_name);
	} QueryVariableMemberMap;

	typedef struct {
		const char *tableName;
		const char *database; //can be null
		int num_members;
		QueryVariableMemberMap *variable_map;
		void *(*mpFactoryMethod)(DataRow *record);
		sGenericData *(*mpGetDataByNameFunc)(const char *variable_name);
	} QueryableClassDesc;

	/////////////////////////////////////////////////
	///// Data Query builder & executor interface
	class DataQuery {
		public:
			DataQuery(QueryableClassDesc *class_desc);
			virtual DataRow* select(int pk_id) = 0;
			virtual DataResultSet* select(QuerySearchParams *search_params = NULL) = 0;
			virtual DataRow* remove(int pk_id) = 0;
			virtual DataResultSet* remove(QuerySearchParams *search_params) = 0;
		protected:
			QueryableClassDesc *mp_class_desc;
	};

	class DataSourceLinkedClass {
		public:
			DataSourceLinkedClass(DataRow *record) { mp_record = record; };
			virtual ~DataSourceLinkedClass() { };
			virtual void remove() = 0;
			virtual void save() = 0;
			virtual void repull() = 0;
			virtual void lock() = 0;
			virtual void unlock() = 0;
		private:
			DataRow *mp_record;
	};

	////////////////////////////////////
	//// Database Connection interface
	class DataSource {
		public:
			DataSource();
			virtual ~DataSource();
			virtual void connect(const char *username, const char *password, const char *server, const char *database) = 0;	
			virtual void disconnect() = 0;
			virtual DataQuery* makeSelectQuery(QueryableClassDesc *class_desc, QuerySearchParams *params) = 0;
	};
}
#endif //_DATASOURCE_H