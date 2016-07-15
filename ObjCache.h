#include <stdlib.h>
#include "Map.h"
namespace Core {

	template<typename T, typename IdentType>
	class CachedObjectManager {
	public:
		CachedObjectManager() : mp_object_list(666) {
			printf("Cache mgr\n");
		}
		/*
			Request or create object
		*/
		T RequestInstance(IdentType identifier){
				return mp_object_list[identifier];
		}
		void InsertInstance(IdentType identifier, T val) {
			mp_object_list[identifier] = val;
		}

		void ClearInstance(IdentType identifier) {
			if(mp_object_list[identifier] != T()) {
				delete mp_object_list[identifier];
				mp_object_list[identifier] = T();
			}
		}

		void ReplaceInstance(IdentType old, IdentType newType, T val) {
			mp_object_list[newType] = mp_object_list[old];
			mp_object_list[old] = T();
		}

	private:
		Core::Map<IdentType, T> mp_object_list;
	};

	template<typename T, typename IdentType>
	class CachedObject {
	public:
		CachedObject() : m_ident_type(), m_object_inserted(false) {
			printf("New cached object %p\n", this->mp_cache_mgr);
		}

		//set identifier and insert into/delete from object list
		void SetCacheIdentifier(IdentType identifier) {
			if(!m_object_inserted) {
				this->mp_cache_mgr->InsertInstance(identifier, (T)this);
				m_object_inserted = true;
				printf("Inserted from ident: %d\n",identifier);
			}
			m_ident_type = identifier;

		}	
		
		IdentType GetCacheIdentifier() {
			return m_ident_type;	
		}
	protected:
		static CachedObjectManager<T, IdentType> *mp_cache_mgr;
		IdentType m_ident_type;
		bool m_object_inserted;
	};
}