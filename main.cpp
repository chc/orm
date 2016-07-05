#include "DataSource.h"
#include "MySQLDataSource.h"
class User : public DB::DataSourceLinkedClass {
	public:
		User(DB::DataRow *row) : DB::DataSourceLinkedClass(row) {
			printf("New user\n");
		}
		User(DB::DataSource *src) : DB::DataSourceLinkedClass(src) {
			printf("New user\n");
		}
		~User() {

		}
		DB::QueryVariableMemberMap *getPrimaryKey() {
			DB::QueryVariableMemberMap *memberMap;
			int num_members;
			memberMap = this->getMemberMap(num_members);
			DB::QueryVariableMemberMap *pk = getMemberByName("id", memberMap, num_members);
			return pk;
		}
		//queryable class impl
		static DB::QueryableClassDesc *getDesc() {
			return &User::classDesc;
		}
		static void dbsrc_SetUsername(DataSourceLinkedClass *obj, sGenericData *data, const char *variable_name) {
			((User *)obj)->username = strdup(data->sUnion.mString);
		}
		static void dbsrc_SetPassword(DataSourceLinkedClass *obj, sGenericData *data, const char *variable_name) {
			((User *)obj)->password = strdup(data->sUnion.mString);
		}
		static void dbsrc_SetID(DataSourceLinkedClass *obj, sGenericData *data, const char *variable_name) {
			((User *)obj)->id = (data->sUnion.uInt32Data);
		}
		static void* userFactory(DB::DataSource *src) {
			return (void *)new User(src);
		}
		DB::QueryVariableMemberMap *getMemberMap(int &member_map) {
			member_map = User::classDesc.num_members;
			return (DB::QueryVariableMemberMap *)&User::memberMap;
		}
		DB::QueryableClassDesc *getClassDesc() {
			return User::getDesc();
		}
		static sGenericData dbsrc_GetUsername(DataSourceLinkedClass *obj, const char *variable_name) {
			sGenericData data;
			data.type = EDataType_String_ASCII;
			data.sUnion.mString = ((User *)obj)->username;
			return data;
		}
		static sGenericData dbsrc_GetPassword(DataSourceLinkedClass *obj, const char *variable_name) {
			sGenericData data;
			data.type = EDataType_String_ASCII;
			data.sUnion.mString = ((User *)obj)->password;
			return data;
		}
		static sGenericData dbsrc_GetID(DataSourceLinkedClass *obj, const char *variable_name) {
			sGenericData data;
			data.type = EDataType_UInt32;
			data.sUnion.uInt32Data = ((User *)obj)->id;
			return data;
		}
	//private:
		static DB::QueryableClassDesc classDesc;
		static DB::QueryVariableMemberMap memberMap[];
		const char *username;
		const char *password;
		int id;
};

DB::QueryVariableMemberMap User::memberMap[] = {
	{"id", EDataType_UInt32, User::dbsrc_SetID, User::dbsrc_GetID},
	{"username", EDataType_String_ASCII, User::dbsrc_SetUsername, User::dbsrc_GetUsername},
	{"password", EDataType_String_ASCII, User::dbsrc_SetPassword, User::dbsrc_GetPassword},
};
DB::QueryableClassDesc User::classDesc = {"user", "test", 3, (DB::QueryVariableMemberMap *)&User::memberMap, User::userFactory};

int main() {
	DB::QuerySearchParams *where = new DB::QuerySearchParams();
	sGenericData *d1 = (sGenericData *)malloc(sizeof(sGenericData)), *d2 = (sGenericData *)malloc(sizeof(sGenericData));
	where->pushOperator(DB::EQueryOperator_Less);
	//where->pushOperator(DB::EQueryOperator_And);
	d1->type = EDataType_UInt32;
	d2->type = EDataType_VariableName;
	d1->sUnion.uInt32Data = 10;
	d2->sUnion.mString = "id";
	where->pushData(d2);
	where->pushData(d1);
	DB::MySQLDataSource *db = new DB::MySQLDataSource();
	printf("%p!!!\n", db);
	db->connect("root", "123321", "localhost", "test");
	DB::DataQuery *query = db->makeSelectQuery(User::getDesc(), NULL);
	DB::QueryLimit limit;
	limit.offset = 0;
	limit.row_count = 15;
	DB::DataResultSet *res = query->select(where, DB::EQuerySortMode_Ascending, &limit);
	Core::Iterator<Core::Vector<void *>, void *> it = res->begin();
	while(it != res->end()) {
		User *user = (User *)*it;
		printf("User: %d || %s || %s\n", user->id, user->username, user->password);
		user->password = "123321";
		user->save();
		it++;
	}

	query->remove(where);
	delete db;
	return 0;
}
