/********************************************************************
	created:	2011/02/18
	created:	18:2:2011   11:30
	filename: 	d:\Work\server_trunk\Server\SMU\SMU_DataMsg.h
	file path:	d:\Work\server_trunk\Server\SMU
	file base:	SMU_DataMsg
	file ext:	h
	author:		杨文鸽
	
	purpose:	通过共享内存,实现数据消息通讯
*********************************************************************/

#ifndef _INCLUDE_SMU_DATAMSG_H_
#define _INCLUDE_SMU_DATAMSG_H_

#include "ShareMemAO.h"
#include <list>
#include <vector>
#include "TimeManager.h"
#include "EasyStack.h"
#include "Lock.h"

#define SUM_DATA_MSG_MAX_SIZE	(1024 * 100)		//100K 可以传递的消息的最大长度 
#define TRY_LOCK_TIME			20					//尝试加锁的等待时间
#define DEFAULT_TICK_SPACE_TIME	100					//消息管理循环处理间隔时间
#define DEFAULT_MSG_TYPE_COUNT	10					//默认预先开设消息类型个数
//----------------------------------------------------------------------------------------
// 数据消息
//----------------------------------------------------------------------------------------
class DataMsgEventFactory;

class DataMsgEvent
{
	friend class SMU_DataMsgManager;
	friend class DataMsgEventFactory;
	friend class SMU_MsgSender;
	friend class SMU_MsgReceiver;

public:
	DataMsgEvent()
		: mpFactory(NULL)
	{

	}
	virtual ~DataMsgEvent()
	{

	}

public:
	virtual INT	GetID(void) = 0;

	virtual BOOL Run(VOID *userData){ return TRUE; };
	virtual BOOL WriteData(CHAR *data, size_t maxLength){ return TRUE; }
	virtual BOOL ReadData(CHAR *data, size_t msgDataSize,VOID *userData){ return TRUE; }
	virtual VOID SetDataSize(size_t size){}
	virtual INT GetDataSize(VOID) { return 0; }
	virtual BOOL IsReplaceSameSendMsg(VOID){ return FALSE; }

	virtual void InitData(){}

protected:
	DataMsgEventFactory *mpFactory;

};
//----------------------------------------------------------------------------------------

typedef std::list<DataMsgEvent*>	MsgEventList;
//----------------------------------------------------------------------------------------
// 消息队列,内部已经加锁处理
//----------------------------------------------------------------------------------------
class SMU_DataMsgList
{
public:
	~SMU_DataMsgList(VOID)
	{
		Lock();
		for (MsgEventList::iterator it=mMsgEventList.begin(); it!=mMsgEventList.end(); ++it)
		{
			delete *it;
		}
		mMsgEventList.clear();
		UnLock();
	}
public:
	VOID Lock(){ /*mLocker.lock();*/ }
	VOID UnLock(){ /*mLocker.unlock();*/ }

	BOOL Empty(VOID)
	{
		BOOL bEmpty = TRUE;
		Lock();
		bEmpty = (mMsgEventList.empty() ? TRUE : FALSE);
		UnLock();
		return bEmpty;
	}

	VOID PushMsg(DataMsgEvent *msg)
	{
		Lock();

		mMsgEventList.push_back(msg);
		UnLock();
	}

	DataMsgEvent* PopMsg(VOID)
	{
		Lock();
		DataMsgEvent *msg = mMsgEventList.front();
		mMsgEventList.pop_front();
		UnLock();
		return msg;
	}

protected:
	MsgEventList	mMsgEventList;
	CsLock			mLocker;
};
//----------------------------------------------------------------------------------------
// 数据消息工厂
//----------------------------------------------------------------------------------------

class DataMsgEventFactory
{
public:
	virtual ~DataMsgEventFactory()
	{
		while (!mFreeList.empty())
		{
			delete mFreeList.pop();
		}
	}

	virtual DataMsgEvent* _NewMsg() = 0;

	virtual INT GetID(VOID) = 0;
	virtual DataMsgEvent* NewMsg()
	{
		if (!mFreeList.empty())
			return mFreeList.pop();

		DataMsgEvent *pMsg = _NewMsg();
		pMsg->mpFactory = this;
		return pMsg;
	}

	virtual void FreeMsg(DataMsgEvent *pMsg)
	{
		pMsg->InitData();
		mFreeList.push(pMsg);
	}

protected:
	EasyStack<DataMsgEvent*>	mFreeList;
};

//----------------------------------------------------------------------------------------
// 用于共享内存互通数据消息的共享管理, 目的可以高效的在多个进程间传递数据消息
// 用了避免线程同步问题, 一组通讯像SOCKET一样对应一组共享内存,从而可以方便实现进程间消息通讯
// 不可以有多于2个进程拥有相同的一组收发共享内存,即一组收发KEY值只能分配到两个进程
//----------------------------------------------------------------------------------------
class SMU_DataMsgManager
{
public:

	// 数据头结构
	struct  MsgHead
	{
		CHAR		state;
		INT			msgID;
		size_t		msgSize;
	};

public:
	SMU_DataMsgManager(SM_KEY key,CHAR okVal,CHAR noOkVal)
		: mOkVal(okVal)
		, mNoOkVal(noOkVal)
		, mSpaceTime(DEFAULT_TICK_SPACE_TIME)
		, mLastTime(0)
		, mUserData(NULL)
		, mDataMsgVec( DEFAULT_MSG_TYPE_COUNT, NULL )
	{
		mMsgDataMaxSize = SUM_DATA_MSG_MAX_SIZE;
		size_t size = SUM_DATA_MSG_MAX_SIZE + _GetHeadLength();
		mShareMem = new ShareMemAO();
		//??? if (!mShareMem->Attach(key,size&0xffff))
		if (!mShareMem->Attach(key,size))
		{
			mShareMem->Create(key,size);
			mDataPtr = mShareMem->GetDataPtr();
			memset(mDataPtr,0,size);
		}
		else
			mDataPtr = mShareMem->GetDataPtr();
	}
	virtual ~SMU_DataMsgManager()
	{
		for (size_t i=0; i<mDataMsgVec.size(); ++i)
		{
			if (mDataMsgVec[i])
				delete mDataMsgVec[i];
		}

		mDataMsgVec.clear();
		delete mShareMem;
		mShareMem = NULL;
	}

	VOID SetUserData(VOID *userData){ mUserData = userData; }
	VOID* GetUserData(VOID){ return mUserData; }

	VOID SetTickSpaceTime(DWORD spaceTime){ mSpaceTime = (WORD)spaceTime & 0XFFFF; }
	WORD GetTickSpaceTime(VOID){ return mSpaceTime; }

	BOOL IsOk(VOID)
	{
		return _getState()==mOkVal;
	}

protected:
	VOID SetOk( BOOL bOk )
	{
		_getState() = (bOk ? mOkVal:mNoOkVal);
	}

	VOID SetSize(size_t size)
	{
		_getSize() = size;
	}

	size_t GetSize(VOID)
	{
		return _getSize();
	}

	CHAR* GetData(VOID){ return _getDataPtr(); }

	VOID SetData(CHAR *szData, size_t len)
	{
		SetSize(len);
		memcpy(_getDataPtr(),szData,len);
	}

	size_t GetMsgMaxSize(void){ return mMsgDataMaxSize; }

public:
	virtual VOID Tick(VOID) = 0;

	virtual VOID AddSendMsg( DataMsgEvent *msg ){}		
	//virtual VOID RegisterMsgFactory( DataMsgEventFactory *msgFactory ){}
	virtual VOID RegisterMsgFactory( DataMsgEventFactory *msgFactory )
	{
		INT id = msgFactory->GetID();
		if ((INT)mDataMsgVec.size()<=id)
			mDataMsgVec.resize(mDataMsgVec.size()+1,NULL);
		if (mDataMsgVec[id])
			delete mDataMsgVec[id];
		mDataMsgVec[id] = msgFactory;
	}	

	DataMsgEvent* NewMsg(INT id)
	{
		if (id>=0 && id<(INT)mDataMsgVec.size())
			if (mDataMsgVec[id])
				return mDataMsgVec[id]->NewMsg();
		return NULL;
	}

	void FreeMsg(DataMsgEvent *pMsg)
	{
		pMsg->mpFactory->FreeMsg(pMsg);
	}

protected:
	CHAR&	_getState		(VOID){ return _getMsgHead().state; }

	size_t&		_getSize		(VOID){ return _getMsgHead().msgSize; }
	INT&		_getID			(VOID){ return _getMsgHead().msgID; }

	CHAR*		_getDataPtr		(VOID){ return mDataPtr+sizeof(MsgHead); }

	MsgHead& _getMsgHead	(VOID){ return *((MsgHead*)mDataPtr); }

	size_t _GetHeadLength(VOID){ return sizeof(MsgHead); }



protected:
	std::vector<DataMsgEventFactory*>	mDataMsgVec;

protected:
	CHAR			mOkVal;
	CHAR			mNoOkVal;
	WORD			mSpaceTime;
	DWORD			mLastTime;
	HANDLE			mMutexHandle;

	ShareMemAO		*mShareMem;
	CHAR			*mDataPtr;
	size_t			mMsgDataMaxSize;

	VOID			*mUserData;

};
//----------------------------------------------------------------------------------------
// 用于发送的管理
//----------------------------------------------------------------------------------------
class SMU_MsgSender : public SMU_DataMsgManager
{
public:
	SMU_MsgSender(SM_KEY key,CHAR okVal,CHAR noOkVal)
		: SMU_DataMsgManager(key,okVal,noOkVal)
	{
		
	}
	~SMU_MsgSender()
	{
		
	}

	virtual VOID Tick(VOID)
	{
		//if (TimeManager::NowTick()-mLastTime>mSpaceTime)
		{
			//mLastTime = TimeManager::NowTick();
			if (!mMsgLister.Empty())
			{
				if (IsOk())
				{
					DataMsgEvent *msg = mMsgLister.PopMsg();
					_sendMsg(msg);			
					msg->mpFactory->FreeMsg(msg);
				}
				//else
				//	break;
			}
		}
	}

	virtual VOID AddSendMsg( DataMsgEvent	*msg )
	{
		mMsgLister.PushMsg(msg);
	}

	bool HaveSendMsg(){ return !mMsgLister.Empty(); }

public:
	BOOL _sendMsg(DataMsgEvent *msg)
	{
		if (msg->WriteData(_getDataPtr(), msg->GetDataSize()))
		{
			_getID() = msg->GetID();
			_getSize() = msg->GetDataSize();
			SetOk(FALSE);
			//printf("发送:[%d],size:[%u]\n",msg->GetID(),msg->GetDataSize());
			return TRUE;
		}
		else
		{
			printf("XXXXXXXXXX-->消息写入失败!\n");
		}
		return FALSE;
	}

protected:
	SMU_DataMsgList			mMsgLister;

};


//----------------------------------------------------------------------------------------
// 用于接收消息管理
//----------------------------------------------------------------------------------------
class SMU_MsgReceiver : public SMU_DataMsgManager
{
public:
	SMU_MsgReceiver(SM_KEY key, CHAR okVal, CHAR noOkVal )
		: SMU_DataMsgManager(key,okVal,noOkVal)		
	{
		
	}
	~SMU_MsgReceiver()
	{

	}

	virtual VOID Tick(VOID)
	{
		//if (TimeManager::NowTick()-mLastTime>mSpaceTime)
		{
			//mLastTime = TimeManager::NowTick();

			if (IsOk())
			{
				INT id = _getID();
				DataMsgEvent *msg = NewMsg(id);
				if (msg)
				{
					msg->SetDataSize(_getSize());
					//printf("接收:[%d], size:[%u]\n", msg->GetID(), _getSize());
					if (msg->ReadData(_getDataPtr(), _getSize(), GetUserData()))
						OnRecvDataMsg(msg);
					msg->mpFactory->FreeMsg(msg);
				}
				else
				{
					SetOk(FALSE);
					AssertEx(0,"不存在的共享消息类型!");
					return;
				}
				SetOk(FALSE);
			}
		}
	}

	virtual BOOL OnRecvDataMsg(DataMsgEvent *msg){ msg->Run(GetUserData()); return TRUE; }



};

//----------------------------------------------------------------------------------------
// 数据消息工厂定义
//----------------------------------------------------------------------------------------

#define DEFINE_MSG_FACTORY( name, id, msgClass )		\
class name : public DataMsgEventFactory\
{\
public:\
	virtual ~name(){}\
\
	virtual INT GetID(VOID){ return id; }\
	virtual DataMsgEvent* _NewMsg() { return new msgClass; }\
};
//----------------------------------------------------------------------------------------


#endif//_INCLUDE_SMU_DATAMSG_H_