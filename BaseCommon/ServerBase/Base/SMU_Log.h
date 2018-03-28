
#ifndef _INCLUDE_SMULOG_H_
#define _INCLUDE_SMULOG_H_

#include "SMU_DataMsg.h"
#include "LogThread.h"
#include "DataBuffer.h"
#include "LogEvent.h"
//-------------------------------------------------------------------------*/
enum SMU_LOG_MSG
{	
	eSMULogMsg,
};
//-------------------------------------------------------------------------*/
class SMULogMsg : public DataMsgEvent
{
public:
	virtual INT	GetID(void) override { return eSMULogMsg; }

	virtual BOOL WriteData(CHAR *data, size_t maxLength){ mData.seek(0); mData._read(data, mData.dataSize()); return TRUE; }
	virtual BOOL ReadData(CHAR *data, size_t msgDataSize,VOID *userData)
	{
		mData.seek(0);
		mData._write(data, msgDataSize);
		return TRUE; 
	}

	virtual INT GetDataSize(VOID) { return mData.dataSize(); }

	DataBuffer mData;
};


DEFINE_MSG_FACTORY(SMULogMsgFactory, eSMULogMsg, SMULogMsg);
//-------------------------------------------------------------------------*/
class SMULogThread : public LogThreadManager
{
public:
	SMULogThread(int smuID)
		: LogThreadManager()
		, mbOverTime(false)
		, mLastTryTime(0)
	{
		mSUMSender = new SMU_MsgSender(smuID, 0, 'N');
		mSUMSender->RegisterMsgFactory(new SMULogMsgFactory());
	}
	~SMULogThread()
	{
		if (mSUMSender!=NULL)
			delete mSUMSender;
		mSUMSender = NULL;
	}

public:
	virtual void __writeLog(const char *szLog)
	{	
		LogThreadManager::__writeLog(szLog);
		if (mbOverTime)
		{
			if (TimeManager::NowTick()-mLastTryTime>6000)
				mbOverTime = false;
		}
		else
		{		
			SMULogMsg *pMsg = dynamic_cast<SMULogMsg*>(mSUMSender->NewMsg(eSMULogMsg));
			const char *sz = szLog;
			pMsg->mData.seek(0);
			pMsg->mData._write((void*)sz, strlen(sz)+1);
			UInt64 now = TimeManager::NowTick();

			while (true)
			{
				if (mSUMSender->IsOk())
				{
					mSUMSender->_sendMsg(pMsg);
					mSUMSender->FreeMsg(pMsg);
					break;
				}
				TimeManager::Sleep(1);
				if (TimeManager::NowTick()-now>2000)
				{
					mSUMSender->FreeMsg(pMsg);
					mbOverTime = true;
					mLastTryTime = TimeManager::NowTick();
					printf("WARN: 日志进程未打开\n");
					break;
				}
			}
		}
		//mSUMSender->AddSendMsg(pMsg);
		//mSUMSender->Tick();		
	}

	//virtual bool needWait()
	//{
	//	mSUMSender->Tick();
	//	return false; //!mSUMSender->HaveSendMsg(); 
	//}

protected:
	SMU_MsgSender *mSUMSender;

	bool	mbOverTime;
	UInt64	mLastTryTime;
};

//-------------------------------------------------------------------------*/
class SMUThreadLog : public ThreadLog
{
public:
	SMUThreadLog(const char *szFile, const char *mode, int smuID)
		: ThreadLog(new SMULogThread(smuID))
	{
		setFile(szFile, mode);
		mLogThreadManager->InitThread();
	}
};
//-------------------------------------------------------------------------*/
#endif