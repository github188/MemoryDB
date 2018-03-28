
#ifndef _INCLUDE_LOGICDBSERVER_H_
#define _INCLUDE_LOGICDBSERVER_H_

#include "BaseThread.h"
#include "MemoryDB.h"
#include "BaseServer.h"

class MemoryDBNode;
class AccountThread : public BaseThread
{
public:
	AccountThread(const char *threadIndexName);
	~AccountThread();

public:
	void RegisterDBOperate();

	// ͬ��������Ϣ��LG
	void SyncServerInfoToLG();

	// �㲥���浽����LG
	void SyncAnnoInfoToLG(AutoData annoData);

	// �������Ƿ��Ѿ�����
	bool ExistLoginNode(int serverID, UInt64 mainNodeKey);

	// �õ����������LG����
	HandConnect GetServerLGConnect(int serverID);
	HandConnect GetServerLGConnectByIp(int nID, AString ip);

	void OnDBStartSucceed();

public:
	virtual void Process(void*)
	{
		mMemoryDB->Process();
	}
	virtual void OnStart(void*);
		
	virtual void OnStop(void*) {}

	virtual bool NotifyThreadClose();

#if DEVELOP_MODE
	virtual int OnceTime(){ return 1; }	
#else
	virtual int OnceTime(void) const{ return 1; }
	// ��ʽ����ʱȫ�ٴ���
	//virtual void OnOnceLoopEnd(){}
#endif	

	virtual void InitEventCenter(AutoEventCenter eventCenter);

public:
	MemoryDBNode* GetDB();

	void _NewCreateDBTable(const char *szTableName, AutoTable table);

public:
	Hand<MemoryDB>	mMemoryDB;
	UInt64			mDBStartTime;

	AutoTable		mServerInfoTable;				// ������������Ϣ��

	AutoTable		mUserDataTable;
	AutoTable		mNewsFeedTable;
	AutoTable		mAccountTable;

};

class AccountServer : public tBaseServer
{
public:
	virtual bool IsStop();
};

#endif //_INCLUDE_LOGICDBSERVER_H_