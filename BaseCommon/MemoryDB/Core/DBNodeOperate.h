#ifndef _INCLUDE_DBNODEOPERATE_H_
#define _INCLUDE_DBNODEOPERATE_H_

#include "Auto.h"
#include "CEvent.h"

#include "ClientEvent.h"
#include "DBOperate.h"

using namespace Logic;

class MemoryDBNode;

class tDBNodeOpreate : public CEvent
{
public:
	tDBNodeOpreate()
	{
		mResultData = MEM_NEW NiceData(); 
		//mResult = eNoneError;
	}
public:
	virtual void OnStartRun(tNetConnect *pRequestConnect){ mRequstConnect = pRequestConnect; }
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData) = 0;
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData, DBResultCallBack callBack)
	{
		int result = Execute(pDBNode, szTable, recordKey, paramData);
		if (result!=eNoneError)
			mResultData->set("ERROR", (int)result);
		if (result!=eWaitFinish)
			callBack(mResultData.getPtr(), result==eNoneError);
		else
			mCallBack = callBack;
		return result;
	}
	virtual void Finish(int result)
	{
		mCallBack(mResultData.getPtr(), result==eNoneError);
	}

	virtual void InitData() override
	{
		CEvent::InitData();
		mResultData->clear(false);
		mErrorInfo.setNull();
		mRequstConnect.setNull();
	}

public:
	AutoNice			mResultData;
	AString				mErrorInfo;
	DBResultCallBack	mCallBack;
	HandConnect			mRequstConnect;
};
typedef Hand<tDBNodeOpreate>	AutoOpereate;



typedef AutoEventFactory	AutoDBNodeOpreateFactory;

//-------------------------------------------------------------------------*/
// �����쳣��ʵ�ּ�¼�첽���ã���δӦ��
// �˷�������ʹ�ò����߼���д���㣬
// �����ٹ��Ļص����⣬ֻ��Ҫʹ�� GetRecord(t, szKey)
// �����ڸ��ӵĲ���������������, ����ִ�����GetRecordʱ��������ִ��Run����
//-------------------------------------------------------------------------*/
////class LoadException : public std::exception
////{
////public:
////	LoadException(const char *szKey)
////		: mRecordKey(szKey)
////	{
////
////	}
////
////public:
////	AString	mRecordKey;
////};
////
////class AsyncLoader : public CEvent
////{
////public:
////	void OnLoadFinish(AutoRecord re) { Run(); }
////	void _Run() = 0;
////
////	AutoRecord GetRecord(AutoTable t, const char *szKey)
////	{
////		AutoRecord re = t->GetRecord(szKey);
////		if (re)
////		{
////			Hand<AsyncRecord> r = re;
////			r->LoadRecord(szKey, GetSelf());
////		}
////	}
////
////	void Run()
////	{
////		try
////		{
////			_Run();
////		}
////		catch (LoadException &e)
////		{
////
////		}
////		catch (std::Exception &e)
////		{
////		}
////	}
////};

// ʹ���첽��ȡ
////class AsyncRecord
////{
////public:
////	void LoadRecord(const char *szKey, AutoEvent callBack)
////	{
////		if (IsLoaded())
////		{
////			Hand<AsyncRecord> r = callBack;
////			r->OnLoadFinish(GetSelf());
////			return;
////		}
////		else if (!IsLoading())
////		{
////			// Load
////		}
////
////		mCallBack.push_back(callBack);
////		throw LoadException(szKey);
////	}
////
////	bool IsLoading(){ return false; }
////
////	bool IsLoaded(){ return false; }
////
////	void OnLoadFinish()
////	{
////		for (size_t i=0; i<mCallBack.size(); ++i)
////		{
////			Hand<AsyncRecord> r = mCallBack[i];
////			r->OnLoadFinish(GetSelf());
////		}
////		mCallBack.clear();
////	}
////
////public:
////	Array<AutoEvent> mCallBack;
////};
//-------------------------------------------------------------------------*/


#endif //_INCLUDE_DBNODEOPERATE_H_