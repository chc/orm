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

		static Core::Vector<DB::DataSourceLinkedClass *> *dbsrc_GetProfiles(DataSourceLinkedClass *obj, const char *variable_name) {
			return &((User *)obj)->m_profiles;
		}
		static void dbsrc_AppendProfile(DataSourceLinkedClass *obj, const char *variable_name, DataSourceLinkedClass *child) {
			((User *)obj)->m_profiles.add(child);
		}
	//private:
		static DB::QueryableClassDesc classDesc;
		static DB::QueryVariableMemberMap memberMap[];
		static DB::QueryableClassRelationshipDesc relations[];
		const char *username;
		const char *password;
		int id;

		Core::Vector<DB::DataSourceLinkedClass *> m_profiles;
};


class Profile : public DB::DataSourceLinkedClass {
	public:
		Profile(DB::DataRow *row) : DB::DataSourceLinkedClass(row) {
			printf("New profile\n");
			userObj = NULL;
		}
		Profile(DB::DataSource *src) : DB::DataSourceLinkedClass(src) {
			printf("New profile\n");
			userObj = NULL;
		}
		~Profile() {

		}

		DB::QueryVariableMemberMap *getMemberMap(int &member_map) {
			member_map = Profile::classDesc.num_members;
			return (DB::QueryVariableMemberMap *)&Profile::memberMap;
		}
		DB::QueryableClassDesc *getClassDesc() {
			return Profile::getDesc();
		}
		static DB::QueryableClassDesc *getDesc() {
			return &Profile::classDesc;
		}

		static void dbsrc_SetUsername(DataSourceLinkedClass *obj, sGenericData *data, const char *variable_name) {
			((Profile *)obj)->username = strdup(data->sUnion.mString);
		}
		static void dbsrc_SetID(DataSourceLinkedClass *obj, sGenericData *data, const char *variable_name) {
			((Profile *)obj)->id = (data->sUnion.uInt32Data);
		}
		static void dbsrc_SetUserID(DataSourceLinkedClass *obj, sGenericData *data, const char *variable_name) {
			((Profile *)obj)->user_id = (data->sUnion.uInt32Data);
		}

		static void* profileFactory(DB::DataSource *src) {
			return (void *)new Profile(src);
		}
		static sGenericData dbsrc_GetUsername(DataSourceLinkedClass *obj, const char *variable_name) {
			sGenericData data;
			data.type = EDataType_String_ASCII;
			data.sUnion.mString = ((Profile *)obj)->username;
			return data;
		}
		static sGenericData dbsrc_GetID(DataSourceLinkedClass *obj, const char *variable_name) {
			sGenericData data;
			data.type = EDataType_UInt32;
			data.sUnion.uInt32Data = ((Profile *)obj)->id;
			return data;
		}

		static sGenericData dbsrc_GetUserID(DataSourceLinkedClass *obj, const char *variable_name) {
			sGenericData data;
			data.type = EDataType_UInt32;
			data.sUnion.uInt32Data = ((Profile *)obj)->user_id;
			return data;
		}

		static void dbsrc_SetUser(DataSourceLinkedClass *obj, sGenericData *data, const char *variable_name) {
			((Profile *)obj)->userObj = (User *)(data->sUnion.pVoidPtr);
		}

		static DB::QueryableClassDesc classDesc;
		static DB::QueryVariableMemberMap memberMap[];
		static DB::QueryableClassRelationshipDesc relationships[];

		int id;
		const char *username;
		User *userObj;
		int user_id;
};


DB::QueryVariableMemberMap User::memberMap[] = {
	{"id", EDataType_UInt32, User::dbsrc_SetID, User::dbsrc_GetID},
	{"username", EDataType_String_ASCII, User::dbsrc_SetUsername, User::dbsrc_GetUsername},
	{"password", EDataType_String_ASCII, User::dbsrc_SetPassword, User::dbsrc_GetPassword},
};
DB::QueryableClassRelationshipDesc User::relations[] = {
	{"id", "user_id", DB::ERelationshipType_OneToMany, NULL, NULL, &Profile::classDesc},
};
DB::QueryableClassDesc User::classDesc = {"user", "test", 3, (DB::QueryVariableMemberMap *)&User::memberMap, 1, (DB::QueryableClassRelationshipDesc *)&User::relations, User::userFactory};


DB::QueryVariableMemberMap Profile::memberMap[] = {
	{"id", EDataType_UInt32, Profile::dbsrc_SetID, Profile::dbsrc_GetID},
	{"username", EDataType_String_ASCII, Profile::dbsrc_SetUsername, Profile::dbsrc_GetUsername},
	{"user_id", EDataType_UInt32, Profile::dbsrc_SetUserID, Profile::dbsrc_GetUserID},
};

DB::QueryableClassRelationshipDesc Profile::relationships[] = {
	{"user_id", "id", DB::ERelationshipType_OneToOne, Profile::dbsrc_SetUser, NULL, &User::classDesc},
};
DB::QueryableClassDesc Profile::classDesc = {"profile", "test", 3, (DB::QueryVariableMemberMap *)&Profile::memberMap, 1 ,(DB::QueryableClassRelationshipDesc *)&Profile::relationships, Profile::profileFactory};

int main() {
	DB::QuerySearchParams *where = new DB::QuerySearchParams();
	sGenericData *d1 = (sGenericData *)malloc(sizeof(sGenericData)), *d2 = (sGenericData *)malloc(sizeof(sGenericData));
	//user query tester
	where->pushOperator(DB::EQueryOperator_Less);
	//where->pushOperator(DB::EQueryOperator_And);
	d1->type = EDataType_UInt32;
	d2->type = EDataType_VariableName;
	d1->sUnion.uInt32Data = 10;
	d2->sUnion.mString = "id"; //getFullColumnFromVariable(Profile::getDesc(), User::memberMap[0]) << for when profile is primary table and selecting user id
	where->pushData(d2);
	where->pushData(d1);
	DB::MySQLDataSource *db = new DB::MySQLDataSource();
	printf("%p!!!\n", db);
	db->connect("root", "123321", "localhost", "test");
	DB::DataQuery *query = db->makeSelectQuery(User::getDesc(), NULL);
	DB::QueryLimit limit;
	DB::QueryOrder order;
	order.sort = DB::EQuerySortMode_Ascending;
	order.column = (DB::QueryVariableMemberMap *)&User::memberMap[0];
	limit.offset = 0;
	limit.row_count = 15;
	DB::DataResultSet *res = query->select(where, &order, &limit);
	Core::Iterator<Core::Vector<void *>, void *> it = res->begin();
	while(it != res->end()) {
		User *user = (User *)*it;
		printf("User: %d || %s || %s\n", user->id, user->username, user->password);
		user->password = "123321";
		user->save();
		it++;
	}
	query->remove(where);
	delete where;
	//profile
	query = db->makeSelectQuery(Profile::getDesc(), NULL);
	printf("Profile query: %p\n", query);
	res = query->select(NULL, &order, &limit);
	printf("Profile res: %p\n", res);
	Core::Iterator<Core::Vector<void *>, void *> it2 = res->begin();
	while(it2 != res->end()) {
		Profile *profile = (Profile *)*it2;
		printf("Profile: %d || %s || %d || %p\n",profile->id, profile->username, profile->user_id, profile->userObj);
		it2++;
	}
	delete db;
	return 0;
}
