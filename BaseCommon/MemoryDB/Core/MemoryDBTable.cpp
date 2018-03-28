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
				// ͬ����DB
				AString key = re->get(mMainIndexCol);

				if (!mDataSource->SaveRecordData(key.c_str(), re.getPtr()))				
				{
					ERROR_LOG("��¼[%s]���浽����Դʧ��", re->getIndexData().string().c_str());
				}
				OnRecordTrigger(re);
				re->FullAllUpdate(false);
			}
		}
		return;
	}

	OnLoopAllEnd();
	// ֻ������Ҫͬ����DB�ļ�¼
	UInt64 nowTime = TimeManager::NowTick();

	if (nowTime-mLastProcessTime<MEMORY_TABLE_PROCESS_SPACE_TIME)
		return;

	mLastProcessTime = nowTime;

	if (!mUpdateList.empty())
	{
		mNowUpdateDB = true;
		for (LoopPool<AutoRecord>::iterator it=mUpdateList.begin(); it; )
		{
			// ��̨������������, ��Чƽ��
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
						// ͬ����DB
						AString key = re->get(mMainIndexCol);

						if (!mDataSource->SaveRecordData(key.c_str(), re.getPtr()))				
						{
							ERROR_LOG("��¼[%s]���浽����Դʧ��", re->getIndexData().string().c_str());
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

	// ָ��ÿ��ѭ������
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
////					// ͬ����DB
////					Data key = re->get(mMainIndexCol);
////
////					if (!mDataSource->SaveRecordData(key.c_str(), re.getPtr()))				
////					{
////						ERROR_LOG("��¼[%s]���浽����Դʧ��", re->getIndexData().c_str());
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
	// ���Լ��� 100������ʱ��
	//for (int i=0; i<1000000; ++i)
	//{
	//	ARecord re = CreateRecord((Int64)i, true);
	//	re->FullAllUpdate(true);
	//}
	//TABLE_LOG("100�� time [%u] s", TimeManager::Now()-mBeginTime);
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
	// NOTE:  ʹ�÷ֲ����, ʹ��ID�Ż���ʹ��KEY�ڴ� �����˷�, �Ҷ����ɴ���KEY��������, ��ǰֹͣʹ�� 2017.09.14
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
	// �޸�Ϊ��ϣslot ��ʽ
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
	//		//!!! NOTE: ��ǰ�����ַ����������зֱ���, �ַ������ֻ��ʹ�õ��ڵ�DB
	//		return true;
	//		//keyID = MAKE_INDEX_ID(key);
	//	}
	//	else
	//		keyID = TOINT64(key);

	//	return keyID>=mMinKey && keyID<=mMaxKey;
	//}
	//TABLE_LOG("WARN: ��δ����������");
	return false;
}

void MemoryDBTable::OnDeleteRecord( const char *szKey, const char *, int deletedCount )
{
	if (deletedCount<=0)
	{
		ERROR_LOG("ɾ��ʧ�� [%s] in table [%s]", szKey, GetTableName());
	}
}

bool MemoryDBTable::DeleteRecord( ARecord record )
{
	if (RemoveRecord(record))
	{
		// �Ӵ洢����ɾ��,
		// NOTE: �첽ʱ, ���뱣֤Ͷ��ִ��˳��, ��ɾ�����ٲ���ͬ��KEY��ֵʱ, �Ų��������ɾ�µļ�¼
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
		// NOTE:  ʹ�÷ֲ����, ������KEY, �����Ƕ�Ӧ�Ĺ�ϣ��
		Int64 newKey = mLastMaxKey + 1;
		if (!mKeyHashSlotList.empty())
		{		
			short hashSlot = HASH_SLOT(newKey);
			if (!mKeyHashSlotList.exist(hashSlot))
			{
				// ���䵽SLOT��Χ��
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
				ERROR_LOG("�ָ���¼����ʧ��");
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
		//			ERROR_LOG("�ָ���¼����ʧ��");
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
		//	ERROR_LOG("ERROR: [%s]������������¼ʧ��>ID����", GetTableName());
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
	//	ERROR_LOG("[%s]��ʼ��������ʧ��", t->GetTableName());

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
	// ׷�ӵ����֮��Ž������ø���
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
		// ���ʹ�ú�̨������������, ������Ҫ����ʱ��
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
		WARN_LOG("�޸�DB��¼����ֵ [%lld]>[%s]", scrValue, re->getIndexData().string().c_str());
		if (mMainKeyIndex->GetRecord(scrValue)==re && mMainKeyIndex->RemoveRecord(scrValue))
		{
			Auto<MemoryDBRecord> r = re;
			mDataSource->DelectData(STRING(scrValue), r->mExtValue, LoadDataCallBack(&MemoryDBTable::OnDeleteRecord, this) );
			r->mExtValue = 0;
			r->SetNewInsert(true);
			if (!AppendRecord(re, false))			
			{
				ERROR_LOG("��������ʱ�������¼ʧ��, ���ܵ�ǰ�Ѿ���������>[%s]", re->getIndexData().string().c_str());				
			}
		}	
		else
			WARN_LOG("ԭ��¼�Ƴ�ʧ��>[%lld]", scrValue);
	}
}

void MemoryDBTable::OnRecordDataChanged(ARecord re, int col, const char *scrValue)
{
	if (col==GetMainIndexCol())
	{	
		WARN_LOG("�޸�DB��¼����ֵ [%s]>[%s]", scrValue, re->getIndexData().string().c_str());
		if (mMainKeyIndex->GetRecord(scrValue)==re && mMainKeyIndex->RemoveRecord(scrValue))
		{
			Auto<MemoryDBRecord> r = re;
			mDataSource->DelectData(scrValue, r->mExtValue, LoadDataCallBack(&MemoryDBTable::OnDeleteRecord, this) );
			r->mExtValue = 0;
			r->SetNewInsert(true);
			if (!AppendRecord(re, false))			
			{
				ERROR_LOG("��������ʱ�������¼ʧ��, ���ܵ�ǰ�Ѿ���������>[%s]", re->getIndexData().string().c_str());				
			}
		}
		else
			WARN_LOG("ԭ��¼�Ƴ�ʧ��>[%s]", scrValue);
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
	// DB �ڴ�������ʹ��STL��Ŀ�Ŀ���
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
			return MEM_NEW IntEasyHashIndex();			// NOTE: ��ID �����ֶ�ʱ, ʹ��EasyHash<int, ARecord> 2017.6.20
		else
			return MEM_NEW Int64MapIndex();

	case FIELD_UINT64:
		if (bMultKey)
			return MEM_NEW MultipleInt64MapIndex();
		else if (bHash)
			return MEM_NEW Int64HashIndex();  //NOTE: ��ID �����ֶ�ʱ, ʹ��EasyHash<Int64, ARecord>		2017.6.20
		else
			return MEM_NEW Int64MapIndex();

	default:
		{
			FieldInfo info = FieldInfoManager::getFieldInfo(indexKeyType);
			if (info!=NULL)
			{
				TABLE_LOG("ERROR: δʵ�� [%s] ���͵�����", info->getTypeString() );
			}
			else
			{
				TABLE_LOG("ERROR: �������ַ��ֶ����� [%d]", indexKeyType);
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

