
#include "BaseMemoryDB.h"
#include "MemoryDBTable.h"

BaseMemoryDB::~BaseMemoryDB()
{
	for (auto it=mTableList.begin(); it.have(); it.next())
	{
		AutoTable &t = it.get();
		if (t)
		{
			t->ClearAll();
			t._free();
		}
	}
	mTableList.clear(true);
}

//ABaseTable BaseMemoryDB::CreateTable( const char *szIndex )
//{
//	ABaseTable t = MEM_NEW MemoryDBTable((MemoryDB*)this); //tBaseTable::NewBaseTable();
//	t->SetTableName(szIndex);
//	mTableList.erase(szIndex);
//	mTableList.insert(szIndex, t);
//	return t;
//}

ABaseTable BaseMemoryDB::GetTable( const char *szIndex )
{
	return mTableList.find(szIndex);
}

void BaseMemoryDB::Close()
{
	//for (size_t i=0; i<mTableList.size(); ++i)
	//{
	//	if (mTableList.get(i))
	//	{
	//		mTableList.get(i)->ClearAll();
	//		mTableList.get(i)._free();
	//	}
	//}
	//mTableList.clear(true);
}
