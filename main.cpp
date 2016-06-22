#include "DataSource.h"
#include "MySQLDataSource.h"
class User : public DB::DataSourceLinkedClass {
	public:
		User(DB::DataRow *row) : DB::DataSourceLinkedClass(row) {
			printf("New user\n");
		}
		~User() {

		}
		//queryable class impl
		static DB::QueryableClassDesc *getDesc() {
			return &User::classDesc;
		}

		void remove() {

		}
		void save() {

		}
		void repull() {

		}
		void lock() {

		}
		void unlock() {

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
		static void* userFactory(DB::DataRow *rec) {
			return (void *)new User(rec);
		}

	//private:
		static DB::QueryableClassDesc classDesc;
		static DB::QueryVariableMemberMap memberMap[];
		const char *username;
		const char *password;
		int id;
};
DB::QueryVariableMemberMap User::memberMap[] = {
	{"id", EDataType_UInt32, User::dbsrc_SetID},
	{"username", EDataType_String_ASCII, User::dbsrc_SetUsername},
	{"password", EDataType_String_ASCII, User::dbsrc_SetPassword},
};
DB::QueryableClassDesc User::classDesc = {"user", "test", 3, (DB::QueryVariableMemberMap *)&User::memberMap, User::userFactory};

int main() {
	DB::MySQLDataSource *db = new DB::MySQLDataSource();
	printf("%p!!!\n", db);
	db->connect("root", "123321", "localhost", "test");
	DB::DataQuery *query = db->makeSelectQuery(User::getDesc(), NULL);
	DB::DataResultSet *res = query->select();
	Core::Iterator<Core::Vector<void *>, void *> it = res->begin();
	while(it != res->end()) {
		User *user = (User *)*it;
		printf("User: %d || %s || %s\n", user->id, user->username, user->password);
		it++;
	}
	delete db;
	return 0;
}