#include "MemoryDBTable.h"
#include "TimeManager.h"

#include "DataSource.h"
#include "MemoryDB.h"
#include "StringHashIndex.h"
#include "Int64MapIndex.h"
#include "MultipleInt64MapIndex.h"
#include "MultipleStringIndex.h"
#include "DBTool.h"
#include "MemoryTableFieldIndex.h"
#include "IntEasyHashIndex.h"
#include "Int64HashIndex.h"

MemoryDBTable::MemoryDBTable(MemoryDB *pDB) 
	: SkipBaseTable(eInitPoolField)
	, mMainIndexCol(0)
	, mDataSource(NULL)
	, mDB(pDB)
#if MEMORY_DB_DEBUG
	, mTotalUpdateCount(0)
	, mTotalUpdateLoop(0)
	, mBeginTotalTime(TimeManager::NowTick())
#endif	
	, mUpdateList(100000)
	, mNowUpdateDB(false)
	, mStartUpdatePool(true)
	, mLastProcessTime(0)
	, mLastMaxKey(0)
	, mLoadLimitRange(0)
{
#if MEMORY_DB_DEBUG
	mBeginTime = TimeManager::Now();
#endif
}

void MemoryDBTable::Process( int nCount )
{
	if (mDataSource==NULL)
		return;

	mDataSource->Process();

	if (!mMainKeyIndex)
		return;

	if (nCount<=0)
	{
		for (ARecordIt it = mMainKeyIndex->GetRecordIt(); *it; ++(*it))
		{
			ARecord re = *it;
			if (re && re->NeedUpdate())
			{	
				// 同步到DB
				AString key = re->get(mMainIndexCol);

				if (!mDataSource->SaveRecordData(key.c_str(), re.getPtr()))				
				{
					ERROR_LOG("记录[%s]保存到数据源失败", re->getIndexData().string().c_str());
				}
				OnRecordTrigger(re);
				re->FullAllUpdate(false);
			}
		}
		return;
	}

	OnLoopAllEnd();
	// 只更新需要同步到DB的记录
	UInt64 nowTime = TimeManager::NowTick();

	if (nowTime-mLastProcessTime<MEMORY_TABLE_PROCESS_SPACE_TIME)
		return;

	mLastProcessTime = nowTime;

	if (!mUpdateList.empty())
	{
		mNowUpdateDB = true;
		for (LoopPool<AutoRecord>::iterator it=mUpdateList.begin(); it; )
		{
			// 后台操作数量限制, 高效平衡
			if (mDataSource->GetOperateCount()>DATASCOURCE_OPERATE_ALLOW_COUNT)
				break;

			Auto<MemoryDBRecord> re = *it;
			if (re)
			{
				//OnProcessRecord(re.getPtr());
				if ((uint)(nowTime - re->mUpdateTime)>RECORD_UPDATE_TIME)
				{	
					re->mUpdateTime = nowTime;
					if (re->NeedUpdate())
					{			
						// 同步到DB
						AString key = re->get(mMainIndexCol);

						if (!mDataSource->SaveRecordData(key.c_str(), re.getPtr()))				
						{
							ERROR_LOG("记录[%s]保存到数据源失败", re->getIndexData().string().c_str());
						}
						OnRecordTrigger(re);
						re->FullAllUpdate(false);
					}

#if MEMORY_DB_DEBUG
					++mTotalUpdateCount;
#endif
					re->mbInUpdateList = false;
					it.erase();
					continue;
				}

//#if MEMORY_DB_DEBUG
//				++mTotalUpdateCount;
//#endif
			}
			++it;
		}
		mNowUpdateDB = false;
	}

	// 指定每次循环数量
////	if (nCount>mMainKeyIndex->GetCount())
////		nCount = (int)mMainKeyIndex->GetCount();
////
////	if (!mTableRecordIt)
////	{
////		mTableRecordIt = mMainKeyIndex->GetRecordIt();
////		if (! (*mTableRecordIt) )
////			return;
////	}
////
////	uint nowTime = TimeManager::NowTick();
////
////	int nowCount = 0;
////	while (true)
////	{
////		if (mDataSource->GetOperateCount()>DATASCOURCE_OPERATE_ALLOW_COUNT)
////			break;
////
////		if (++nowCount > nCount)
////			break;
////
////        if ( !(*mTableRecordIt) )
////        {			
////            OnLoopAllEnd();
////            mTableRecordIt = mMainKeyIndex->GetRecordIt();
////            if ( !(*mTableRecordIt) )
////                return;
////        }
////
//// 		Hand<MemoryDBRecord> re = (ARecord)(*mTableRecordIt);
////		if (re)
////		{
////			OnProcessRecord(re.getPtr());
////			if (nowTime - re->mUpdateTime>RECORD_UPDATE_TIME)
////			{	
////				re->mUpdateTime = nowTime;
////				if (re->NeedUpdate())
////				{			
////					// 同步到DB
////					Data key = re->get(mMainIndexCol);
////
////					if (!mDataSource->SaveRecordData(key.c_str(), re.getPtr()))				
////					{
////						ERROR_LOG("记录[%s]保存到数据源失败", re->getIndexData().c_str());
////					}
////					re->FullAllUpdate(false);
////				}
////			}
////
////#if MEMORY_DB_DEBUG
////			++mTotalUpdateCount;
////#endif
////
////			++(*mTableRecordIt);
////		}
////	}
}

void MemoryDBTable::OnLoopAllEnd()
{
#if MEMORY_DB_DEBUG
	if (TimeManager::Now()-mBeginTime>10)
	{
		LOG_YELLOW;
		printf("WARN: Loop time over [%d]s\r\n", 10);
		LOG_WHITE;
	}
	mBeginTime = TimeManager::Now();

	static __declspec(thread) int totalNum = 1000000;

	++mTotalUpdateLoop;
	if (mTotalUpdateLoop>=totalNum)
	{
		int useTime = TimeManager::NowTick()-mBeginTotalTime;
		printf("[%s] update %d record, loop [%d] use time [%d] mil secord\r\n", GetTableName(), mTotalUpdateCount, totalNum, useTime);
		mTotalUpdateLoop = 0;
		mTotalUpdateCount = 0;
		mBeginTotalTime = TimeManager::NowTick();
	}
	 
	//if (mMainKeyIndex)
	//	if (mTotalUpdateCount!=mMainKeyIndex->GetCount())
	//	{
	//		TABLE_LOG("WARN: update count is same record count !");
	//	}

	//	mTotalUpdateCount = 0;
#endif
	// 测试加入 100万数据时间
	//for (int i=0; i<1000000; ++i)
	//{
	//	ARecord re = CreateRecord((Int64)i, true);
	//	re->FullAllUpdate(true);
	//}
	//TABLE_LOG("100万 time [%u] s", TimeManager::Now()-mBeginTime);
}

bool MemoryDBTable::SetMainIndex( int indexCol, bool bHash, bool bMultiple )
{
	if (bMultiple)
	{
		ERROR_LOG("MemoryDBTable can not use multiple main index, now alway use only index");
	}
	bool b = BaseTable::SetMainIndex(indexCol, bHash, false);
	if (mMainKeyIndex)
		mTableRecordIt = mMainKeyIndex->GetRecordIt();
	else
		mTableRecordIt.setNull();

	return b;
}

bool MemoryDBTable::SetIDMainIndex(UInt64 minKey, UInt64 maxKey)
{
	// NOTE:  使用分布表格, 使用ID优化会使用KEY内存 有所浪费, 且对自由创建KEY产生局限, 当前停止使用 2017.09.14
	NOTE_LOG("WARN: [%s] DB not use IDArrayIndex, Now use IntEasyHashIndex", GetTableName());
	SetMainIndex(0, true);
	return true;
	//mTableRecordIt.setNull();
	//bool bResult = BaseTable::SetIDMainIndex(minKey, maxKey);
	//if (bResult)
	//	mTableRecordIt = mMainKeyIndex->GetRecordIt();
	//return bResult;
}

bool MemoryDBTable::RemoveRecord( float fIndex )
{
	return RemoveRecord(GetRecord(fIndex));

}

bool MemoryDBTable::RemoveRecord( const char* szIndex )
{
	return RemoveRecord(GetRecord(szIndex));
}

bool MemoryDBTable::RemoveRecord( Int64 nIndex )
{
	return RemoveRecord(GetRecord(nIndex));
}

bool MemoryDBTable::RemoveRecord( ARecord record )
{
	if (!record)
		return false;

	if (mTableRecordIt)
	{		
		if ( mTableRecordIt->GetRecord() == record)
			++(*mTableRecordIt);
	}
	return BaseTable::RemoveRecord(record);
}

bool MemoryDBTable::InKeyRange( const char *key )
{
	// 修改为哈希slot 方式
	if (mKeyHashSlotList.empty())
		return true;

	FieldInfo info = GetField()->getFieldInfo(mMainIndexCol);
	if (info->getType()==FIELD_STRING)
		return mKeyHashSlotList.exist(STRHASH_SLOT(key));

	return mKeyHashSlotList.exist(HASH_SLOT(TOUINT64(key)) );

	//if (mMaxKey<0)
	//	return true;

	//if (mMainKeyIndex)
	//{
	//	Int64 keyID = 0;
	//	if (mMainKeyIndex->IsStringHashKey())
	//	{
	//		//!!! NOTE: 当前不对字符串索引进行分表处理, 字符串表格只能使用单节点DB
	//		return true;
	//		//keyID = MAKE_INDEX_ID(key);
	//	}
	//	else
	//		keyID = TOINT64(key);

	//	return keyID>=mMinKey && keyID<=mMaxKey;
	//}
	//TABLE_LOG("WARN: 还未创建主索引");
	return false;
}

void MemoryDBTable::OnDeleteRecord( const char *szKey, const char *, int deletedCount )
{
	if (deletedCount<=0)
	{
		ERROR_LOG("删除失败 [%s] in table [%s]", szKey, GetTableName());
	}
}

bool MemoryDBTable::DeleteRecord( ARecord record )
{
	if (RemoveRecord(record))
	{
		// 从存储器中删除,
		// NOTE: 异步时, 必须保证投递执行顺序, 当删除后再插入同样KEY的值时, 才不会进行误删新的记录
		Auto<MemoryDBRecord> re = record;
		mDataSource->DelectData(record->getIndexData().string().c_str(), re->mExtValue, LoadDataCallBack(&MemoryDBTable::OnDeleteRecord, this) );
		re->mExtValue = 0;
		return true;
	}
	return false;
}

ARecord MemoryDBTable::GrowthNewRecord( DataStream *recordData )
{
	if (mMainKeyIndex)
	{
		// NOTE:  使用分布表格, 产生的KEY, 必须是对应的哈希槽
		Int64 newKey = mLastMaxKey + 1;
		if (!mKeyHashSlotList.empty())
		{		
			short hashSlot = HASH_SLOT(newKey);
			if (!mKeyHashSlotList.exist(hashSlot))
			{
				// 分配到SLOT范围内
				bool bHave = false;
				for (int i=0; i<mKeyHashSlotList.size(); ++i)
				{
					if (mKeyHashSlotList.getKey(i)>hashSlot)
					{
						bHave = true;
						newKey += mKeyHashSlotList.getKey(i)-hashSlot;
						break;
					}
				}
				if (!bHave)
				{
					newKey += mKeyHashSlotList.getKey(0)+DB_HASH_SLOT_COUNT - hashSlot;
				}
			}
		}
		Auto<MemoryDBRecord> re = NewRecord();
		if (recordData!=NULL)
		{
			recordData->seek(0);
			if (!re->restoreData(recordData))
			{
				ERROR_LOG("恢复记录数据失败");
			}
		}
		re[mMainIndexCol] = newKey;
		if (AppendRecord(re, false))
		{
			re->SetNewInsert(true);
			re->FullAllUpdate(true);
			return re;
		}

		//if (mMaxKey==-1 || mMainKeyIndex->GetNextGrowthKey()<=mMaxKey)
		//{
		//	Auto<MemoryDBRecord> re = NewRecord();
		//	if (recordData!=NULL)
		//	{
		//		recordData->seek(0);
		//		if (!re->restoreData(recordData))
		//		{
		//			ERROR_LOG("恢复记录数据失败");
		//		}
		//	}
		//	if (mMainKeyIndex->InsertLast(re))
		//	{
		//		re->SetNewInsert(true);
		//		re->FullAllUpdate(true);
		//		return re;
		//	}

		//}
		//else
		//{
		//	ERROR_LOG("ERROR: [%s]自增长创建记录失败>ID已满", GetTableName());
		//}
	}
	return ARecord();
}

void MemoryDBTable::ReadyDataSource( tDataSource *pSource, NiceData &initParam )
{
	if (mDataSource)
		delete mDataSource;
	mDataSource = pSource;


	mMainIndexCol = GetField()->getFieldCol(mMainKeyIndex->IndexFieldName());
	FieldInfo info = GetField()->getFieldInfo(mMainKeyIndex->IndexFieldName());
}

MemoryDBTable::~MemoryDBTable()
{
	if (mDataSource!=NULL)
		delete mDataSource;
	mDataSource = NULL;
}

void MemoryDBTable::OnLoadTableAllRecord(const char*, const char*, int) //  DBOperate *op, bool bSu )
{
	ABaseTable t = GetSelf();
	//if (bSu)
	{			
		//Int64 minKey=0, maxKey=0;
		//t->GetKeyIDRange(minKey, maxKey);
		LOG_GREEN;
		NOTE_LOG("[%s] Succeed load all record type>[%s], count = [%llu]", 
			t->GetTableName(), 
			t->GetTableType(), 
			t->GetMainIndex()->GetCount()
			);
		LOG_WHITE;
		mDB->OnNotifyFinishLoadAllRecord(GetSelf());
		SetStartUpdatePool(true);

        //AString log;
        //log.Format("d:/%s.txt", GetTableName());
        //SaveCSV(log.c_str(), true);
	}
	//else
	//	ERROR_LOG("[%s]初始加载数据失败", t->GetTableName());

}

bool MemoryDBTable::LoadAllRecord()
{
	return mDataSource->LoadAllRecord(GetSelf(), LoadDataCallBack(&MemoryDBTable::OnLoadTableAllRecord, this));
}
//-------------------------------------------------------------------------
ARecord MemoryDBTable::NewRecord()
{
	if (!mDB->GetmemoryState())
		throw std::exception("Memory unenough, then new record fail");

	AssertEx(GetField(), "Now table must exist FieldIndex");
	ARecord freeRe = GetField()->TryGetFreeRecord();
	MemoryDBRecord *p = dynamic_cast<MemoryDBRecord*>(freeRe.getPtr());
	if (p!=NULL)	
		p->InitTablePtr(mOwnerPtr);	
	else
	{
		p = MEM_NEW MemoryDBRecord(GetSelf());
		p->_alloctData(0);
	}
	// 追加到表格之后才进行设置更新
	//p->FullAllUpdate(true);
	return p;
}

void MemoryDBTable::ClearAll()
{
	if (mDataSource!=NULL)
		mDataSource->ClearAuto();

	BaseTable::ClearAll();
}

void MemoryDBTable::AppendUpdateRecord( BaseRecord *pNeedUpdateRecord )
{
	if (mStartUpdatePool && !mNowUpdateDB)
	{		
		// 如果使用后台任务数量限制, 不再需要重置时间
		//Hand<MemoryDBRecord> re = pNeedUpdateRecord;
		//re->mUpdateTime = TimeManager::NowTick();		
		MemoryDBRecord *pRe = dynamic_cast<MemoryDBRecord*>(pNeedUpdateRecord);
		if (!pRe->mbInUpdateList)
		{
			pRe->mbInUpdateList = true;
			mUpdateList.insert(pNeedUpdateRecord->GetSelf());
		}
	}
}

bool MemoryDBTable::AppendRecord( ARecord scrRecord, bool bReplace, bool bRestore )
{
	if (SkipBaseTable::AppendRecord(scrRecord, bReplace))
	{
		Auto<MemoryDBRecord> re = scrRecord;
		if (re->IsNewInsert())
			re->FullAllUpdate(true);
		
		Int64 x = re->getIndexData();
		if (x>mLastMaxKey)
			mLastMaxKey = x;
		return true;
	}
	return false;
}

void MemoryDBTable::SaveRecordData( const char *szKey, NiceData &recordData, bool bNewInsert, DBResultCallBack callBack )
{
	AutoField field = GetField(); 
	ARecord re = GetRecord(szKey);
	if (!re)
	{
		callBack(NULL, false);
		return;
	}
	for (auto it=recordData.begin(); it->have(); it->next())
	{
		const EasyString &key = it->key();

		FieldInfo info = field->getFieldInfo(key.c_str());
		if (info==NULL)
		{
			ERROR_LOG("Field [%s] no exist at table [%s]", key.c_str(), GetTableName());
			callBack(NULL, false);
			return;
		}

		if (!re->set(key.c_str(), it->get().get()))
		{
			ERROR_LOG("Set record data fail [%s], type [%s]", key.c_str(), info->getTypeString());
			callBack(NULL, false);
			return;
		}		
	}

	callBack(NULL, true);
}

void MemoryDBTable::LoadRecordData( const char *szKey, AString fieldInfo, DBResultCallBack callBack )
{
	ARecord re = GetRecord(szKey);
	if (!re)
	{
		callBack(NULL, false);
		return;
	}
	Array<AString> strList;
	AString::Split(fieldInfo.c_str(), strList, " ", 100);

	AutoField field = GetField();

	AutoNice resultData = MEM_NEW NiceData();
	for (size_t i=0; i<strList.size(); ++i)
	{
		FieldInfo info = field->getFieldInfo(strList[i].c_str());
		if (info==NULL)
		{
			ERROR_LOG("Field [%s] no exist at table [%s]", strList[i].c_str(), GetTableName());
			callBack(NULL, false);
			return;
		}

		bool b = resultData->getOrCreate(info->getName().c_str()).set(re->get(info->getName().c_str()));

		if (!b)
		{
			ERROR_LOG("Set record data fail [%s], type [%s]", info->getName(), info->getTypeString());
			callBack(NULL, false);
			return;
		}		
		
	}
	callBack(resultData, true);
}

void MemoryDBTable::OnRecordDataChanged(ARecord re, int col, Int64 scrValue)
{
	if (col==GetMainIndexCol())
	{	
		WARN_LOG("修改DB记录主键值 [%lld]>[%s]", scrValue, re->getIndexData().string().c_str());
		if (mMainKeyIndex->GetRecord(scrValue)==re && mMainKeyIndex->RemoveRecord(scrValue))
		{
			Auto<MemoryDBRecord> r = re;
			mDataSource->DelectData(STRING(scrValue), r->mExtValue, LoadDataCallBack(&MemoryDBTable::OnDeleteRecord, this) );
			r->mExtValue = 0;
			r->SetNewInsert(true);
			if (!AppendRecord(re, false))			
			{
				ERROR_LOG("更改主键时，插入记录失败, 可能当前已经存在主键>[%s]", re->getIndexData().string().c_str());				
			}
		}	
		else
			WARN_LOG("原记录移除失败>[%lld]", scrValue);
	}
}

void MemoryDBTable::OnRecordDataChanged(ARecord re, int col, const char *scrValue)
{
	if (col==GetMainIndexCol())
	{	
		WARN_LOG("修改DB记录主键值 [%s]>[%s]", scrValue, re->getIndexData().string().c_str());
		if (mMainKeyIndex->GetRecord(scrValue)==re && mMainKeyIndex->RemoveRecord(scrValue))
		{
			Auto<MemoryDBRecord> r = re;
			mDataSource->DelectData(scrValue, r->mExtValue, LoadDataCallBack(&MemoryDBTable::OnDeleteRecord, this) );
			r->mExtValue = 0;
			r->SetNewInsert(true);
			if (!AppendRecord(re, false))			
			{
				ERROR_LOG("更改主键时，插入记录失败, 可能当前已经存在主键>[%s]", re->getIndexData().string().c_str());				
			}
		}
		else
			WARN_LOG("原记录移除失败>[%s]", scrValue);
	}
}

void MemoryDBTable::OnRecordTrigger(AutoRecord updateRecord)
{
	if (!mRecordTrigger)
		return;

	mRecordTrigger->OnTrigger(updateRecord);	
}

bool MemoryDBTable::RegisterTrigger(HandTrigger trigger, bool bAppend)
{
	mRecordTrigger = trigger;
	return true;
}
AutoIndex MemoryDBTable::NewRecordIndex( FIELD_TYPE indexKeyType, bool bHash, bool bMultKey )
{
	// DB 内存表的索引使用STL，目的快速
	switch (indexKeyType)
	{
	//case FIELD_FLOAT:
	case FIELD_STRING:
		if (bMultKey)
		{
			return MEM_NEW STL_MultipleStringIndex();
		}
		if (bHash)
			return MEM_NEW StringHashIndex();		
		else
			return MEM_NEW StringHashIndex();

	case FIELD_BYTE:
	case FIELD_SHORT:
	case FIELD_INT:
		if (bMultKey)
		{
			return MEM_NEW MultipleInt64MapIndex();
		}
		else if (bHash)
			return MEM_NEW IntEasyHashIndex();			// NOTE: 非ID 名称字段时, 使用EasyHash<int, ARecord> 2017.6.20
		else
			return MEM_NEW Int64MapIndex();

	case FIELD_UINT64:
		if (bMultKey)
			return MEM_NEW MultipleInt64MapIndex();
		else if (bHash)
			return MEM_NEW Int64HashIndex();  //NOTE: 非ID 名称字段时, 使用EasyHash<Int64, ARecord>		2017.6.20
		else
			return MEM_NEW Int64MapIndex();

	default:
		{
			FieldInfo info = FieldInfoManager::getFieldInfo(indexKeyType);
			if (info!=NULL)
			{
				TABLE_LOG("ERROR: 未实现 [%s] 类型的索引", info->getTypeString() );
			}
			else
			{
				TABLE_LOG("ERROR: 不存在字符字段类型 [%d]", indexKeyType);
			}
		}
	}

	return AutoIndex();
}

ARecord MemoryDBTable::CreateRecord(const char* szIndex, bool bReplace)
{	
	ARecord existRe = GetRecord(szIndex);
	if (existRe)
	{
		if (bReplace)
		{
			existRe->InitData();
			existRe->_set(0, szIndex);
			existRe->FullAllUpdate(true);
			return existRe;
		}
		return ARecord();
	}

	Auto<MemoryDBRecord> r = SkipBaseTable::CreateRecord(szIndex, true);
	if (r)
		r->SetNewInsert(true);
	return r;
}

ARecord MemoryDBTable::CreateRecord(Int64 nIndex, bool bReplace)
{	
	ARecord existRe = GetRecord((Int64)nIndex);
	if (existRe)
	{
		if (bReplace)
		{
			existRe->InitData();
			existRe->_set(0, (Int64)nIndex);
			existRe->FullAllUpdate(true);
			return existRe;
		}
		return ARecord();
	}
	
	Auto<MemoryDBRecord> r = SkipBaseTable::CreateRecord((Int64)nIndex, true);
	if (r)
		r->SetNewInsert(true);
	return r;
}

bool MemoryDBTable::LoadFromDB( tDBTool *pDBTool )
{
	ARecord r = NewRecord();
	if (pDBTool->LoadRecord(r))
	{			
		AppendRecord(r, true);
		r->FullAllUpdate(false);
		return true;
	}
	return false;
}



//-------------------------------------------------------------------------

