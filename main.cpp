#include "DataSource.h"
#include "MySQLDataSource.h"
class User : public DB::DataSourceLinkedClass {
	public:
		User(DB::DataRow *row) : DB::DataSourceLinkedClass(row) {
			printf("New user\n");
		}
		~User() {

		}
		const char *getName();
		const char *getPass();
		void setPassword(const char *pass);
		void setUser(const char *user);

		//queryable class impl
		const char *getTableName() { return "user"; };
		const char *setDatabaseName() { return "test"; };
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
			printf("Set username: %s\n", ((User *)obj)->username);
		}
		static void* userFactory(DB::DataRow *rec) {
			return (void *)new User(rec);
		}
	private:
		static DB::QueryableClassDesc classDesc;
		static DB::QueryVariableMemberMap memberMap[];
		const char *username;
		const char *password;
		int id;
};
DB::QueryVariableMemberMap User::memberMap[] = {
	{"id", EDataType_UInt32, NULL},
	{"username", EDataType_String_ASCII, User::dbsrc_SetUsername},
	{"password", EDataType_String_ASCII, NULL},
};
DB::QueryableClassDesc User::classDesc = {"user", "test", 3, (DB::QueryVariableMemberMap *)&User::memberMap, User::userFactory};

int main() {
	DB::MySQLDataSource *db = new DB::MySQLDataSource();
	printf("%p!!!\n", db);
	db->connect("root", "123321", "localhost", "test");
	DB::DataQuery *query = db->makeSelectQuery(User::getDesc(), NULL);
	query->select();
	printf("Query ptr: %p\n", query);
	delete db;
	return 0;
}