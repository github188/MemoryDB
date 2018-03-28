
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
		ERROR_LOG("��ʼ���ؼ�¼DBToolʧ��>[%s]", GetTableName());
	}
}


void CoolDBTable::OnLoadTableAllRecord(const char*, const char*, int) //DBOperate *op, bool bSu )
{
	MemoryDBTable::OnLoadTableAllRecord(NULL, NULL, 0); // op, bSu);

	//if (bSu)
	{
		// ����ѭ�������ȴ�ȴ��¼�
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
	// ׷�ӵ����֮��Ž������ø���
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
		// Ӧ�þ�����ȴ��¼�����½��м���
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
		// Ӧ�þ�����ȴ��¼�����½��м���
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
		// Ӧ�þ�����ȴ��¼�����½��м���
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
				&& re.getUseCount()==2	// NOTE:  Hand ʱ����3, AutoPtr ʱ, ����Ϊ2
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
		//!!! �ڳ�ʼ���غ󣬽��ռ�������ͷ�, �ɸ������ã��Ƿ��ڼ���ʱ������ȫ�����ݣ�������ػ���Ҫ�޸�[DataSource.cpp 589 line]
		r->InitData();
		dynamic_cast<ActiveDBRecord*>(r.getPtr())->_freeData();
		return true;
	}
	return false;	
#endif
	
}
