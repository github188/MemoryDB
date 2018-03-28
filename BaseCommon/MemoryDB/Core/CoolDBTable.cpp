
#include "CoolDBTable.h"
#include "DBTool.h"
#include "MySqlDBTool.h"
#include "MemoryDB.h"
#include "TimeManager.h"
#include "DataSource.h"
//-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
CoolDBTable::CoolDBTable(MemoryDB *pDB)
	: MemoryDBTable(pDB)
	, mCoolTime(DB_COOL_TIME)
{

}

CoolDBTable::~CoolDBTable()
{
	mCheckCoolEvent._free();

}
//-------------------------------------------------------------------------*
void CoolDBTable::ReadyDataSource(tDataSource *pSource, NiceData &initParam)
{
	MemoryDBTable::ReadyDataSource(pSource, initParam);

	if (!pSource->ReadyReloadTool(initParam))
	{
		ERROR_LOG("初始加载记录DBTool失败>[%s]", GetTableName());
	}
}


void CoolDBTable::OnLoadTableAllRecord(const char*, const char*, int) //DBOperate *op, bool bSu )
{
	MemoryDBTable::OnLoadTableAllRecord(NULL, NULL, 0); // op, bSu);

	//if (bSu)
	{
		// 开启循环检查冷却等待事件
		if (mCheckCoolEvent)
			mCheckCoolEvent._free();
		Hand<TM_CoolDBCheckEvent> waitCheck = mDB->GetEventCenter()->StartEvent("TM_CoolDBCheckEvent", false);
		if (!waitCheck)
		{
			mDB->GetEventCenter()->RegisterEvent("TM_CoolDBCheckEvent", MEM_NEW EventFactory<TM_CoolDBCheckEvent>());
			waitCheck = mDB->GetEventCenter()->StartEvent("TM_CoolDBCheckEvent");
		}
		waitCheck->mOwnerTable = this;
		waitCheck->DoEvent();
	}
}

ARecord CoolDBTable::NewRecord() 
{
	AssertEx(GetField(), "Now table must exist FieldIndex");
	ARecord freeRe = GetField()->TryGetFreeRecord();
	ActiveDBRecord *p = dynamic_cast<ActiveDBRecord*>(freeRe.getPtr());
	if (p!=NULL)	
		p->InitTablePtr(mOwnerPtr);	
	else
	{
		p = MEM_NEW ActiveDBRecord(GetSelf());
		p->_alloctData(0);
	}
	
	p->mActiveTime = TimeManager::Now();
	// 追加到表格之后才进行设置更新
	//p->FullAllUpdate(true);
	return p;
}


ARecord CoolDBTable::GetRecord(Int64 nIndex)
{
	ARecord re = MemoryDBTable::GetRecord(nIndex);
	if (!re && re.getPtr()!=NULL)
	{		
		ActiveDBRecord *p = dynamic_cast<ActiveDBRecord*>(re.getPtr());
		UInt64 ext = p->mExtValue;
		// 应该就是冷却记录，重新进行加载
		re->_alloctData(0);
		p->mExtValue = ext;
		if (mDataSource->ReloadRecord(STRING(nIndex), re))
		{		
			p->mActiveTime = TimeManager::Now();
			TABLE_LOG("Succed reload secord >[%lld]", nIndex);
		}
		else
			ERROR_LOG("[%s] Reload record fail>[%s]", GetTableName(), STRING(nIndex));
	}		
	else
	{
		ActiveDBRecord *p = dynamic_cast<ActiveDBRecord*>(re.getPtr());
		if (p!=NULL)
			p->mActiveTime = TimeManager::Now();
	}
	return re;
}

ARecord CoolDBTable::GetRecord(float fIndex)
{
	ARecord re = MemoryDBTable::GetRecord(fIndex);
	if (!re && re.getPtr()!=NULL)
	{		
		ActiveDBRecord *p = dynamic_cast<ActiveDBRecord*>(re.getPtr());
		UInt64 ext = p->mExtValue;
		// 应该就是冷却记录，重新进行加载
		re->_alloctData(0);
		p->mExtValue = ext;
		if (mDataSource->ReloadRecord(STRING(fIndex), re))
		{		
			p->mActiveTime = TimeManager::Now();
			TABLE_LOG("Succed reload secord >[%f]", fIndex);
		}
		else
			ERROR_LOG("[%s] Reload record fail>[%s]", GetTableName(), STRING(fIndex));
	}		
	else
	{
		ActiveDBRecord *p = dynamic_cast<ActiveDBRecord*>(re.getPtr());
		if (p!=NULL)
			p->mActiveTime = TimeManager::Now();
	}
	return re;
}

ARecord CoolDBTable::GetRecord(const char* szIndex)
{
	ARecord re = MemoryDBTable::GetRecord(szIndex);
	if (!re && re.getPtr()!=NULL)
	{		
		ActiveDBRecord *p = dynamic_cast<ActiveDBRecord*>(re.getPtr());
		UInt64 ext = p->mExtValue;
		// 应该就是冷却记录，重新进行加载
		re->_alloctData(0);
		p->mExtValue = ext;
		if (mDataSource->ReloadRecord(szIndex, re))
		{		
			p->mActiveTime = TimeManager::Now();
			TABLE_LOG("Succed reload secord >[%s]", szIndex);
		}
		else
			ERROR_LOG("[%s] Reload record fail>[%s]", GetTableName(), szIndex);
	}		
	else
	{
		ActiveDBRecord *p = dynamic_cast<ActiveDBRecord*>(re.getPtr());
		if (p!=NULL)
			p->mActiveTime = TimeManager::Now();
	}
	return re;
}

void CoolDBTable::CheckCoolRecord()
{
	UInt64 now = TimeManager::Now();
	for (ARecordIt it=GetRecordIt(); *it; ++(*it))
	{
		ARecord re = it->GetRecord();
		if (re)
		{
			ActiveDBRecord *pRe = dynamic_cast<ActiveDBRecord*>(re.getPtr());		
			if (pRe!=NULL && now - pRe->mActiveTime>mCoolTime && !pRe->NeedUpdate() 
				&& re.getUseCount()==2	// NOTE:  Hand 时引用3, AutoPtr 时, 引用为2
				)
			{	
				UInt64 extValue = pRe->mExtValue;
				TABLE_LOG("[%s] >Record [%d] now cooled", GetTableName(), (int)pRe->get(0));
				pRe->InitData();
				pRe->_freeData();
				pRe->mExtValue = extValue;
			}
		}
	}
}

bool CoolDBTable::LoadFromDB( tDBTool *pDBTool )
{
#if COOL_START_LOAD_RECORD
	return MemoryDBTable::LoadFromDB(pDBTool);
#else
	ARecord r = NewRecord();	
	if (pDBTool->LoadRecord(r))
	{			
		AppendRecord(r, true);
		//!!! 在初始加载后，将空间进行了释放, 可根据配置，是否在加载时，加载全部数据，如果加载还需要修改[DataSource.cpp 589 line]
		r->InitData();
		dynamic_cast<ActiveDBRecord*>(r.getPtr())->_freeData();
		return true;
	}
	return false;	
#endif
	
}
