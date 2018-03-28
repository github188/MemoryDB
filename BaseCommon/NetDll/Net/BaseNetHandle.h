#ifndef _INCLUDE_BASENETHANDLE_H_
#define _INCLUDE_BASENETHANDLE_H_

#include "NetHead.h"
#include "NetHandle.h"

class Net_Export tBaseNetHandle : public tNetHandle
{
public:
	tBaseNetHandle(tNetProtocol *pProtocol)
		: mpNetProtocol(pProtocol)
	{
		AssertEx(mpNetProtocol!=NULL, "Must ready net protocol");
	}

	virtual ~tBaseNetHandle()
	{
		if (mpNetProtocol==NULL)
			delete mpNetProtocol;
		mpNetProtocol = NULL;
	}

public:
public:
	virtual void SetIndexName(const char* strIndexName) { mIndexName = strIndexName; }
	virtual const char*  GetIndexName(void) const { return mIndexName.c_str(); }

public:
	//virtual bool StartNet(void) = 0;

	// Define over time 10 second.
	virtual int GetConnectOverTime(void) { return 10; } 
	virtual int GetTryConnectCount(void) { return 6; }
	virtual const char* GetConnectFinishEvent(void) { return NULL; }

public:	
	virtual bool OnReceiveEvent(tNetConnect *revConnect, Logic::tEvent *revEvent);

public:
	//virtual bool Send(Packet *pMsgPacket, tNetConnect *pConnect, ){ return pConnect->Send(pMsgPacket); }
	//virtual bool Send(Logic::tEvent *sendEvent, int target);
	virtual bool Send(Logic::tEvent *sendEvent, tNetConnect *pConnect);

public:
	virtual tNetProtocol* GetNetProtocol(void) { return mpNetProtocol; }


	// 发送和接收, 默认不进行任何转换, 如果对方字符集不一至, 则可以在下面进行转换
	virtual AString OnSendStringConvertCode(const char *szSendString){ return AString(); }
	virtual AString OnReceiveStringConverCode(const char *szReceiveString){		return AString(); }

	// 服务器才需要处理这个加入连接功能
	virtual bool OnAddConnect(tNetConnect *pConnect){ return true; }

	virtual void Log(const char *szInfo, ...);

protected:
	EasyString			mIndexName;
	tNetProtocol		*mpNetProtocol;
};


#endif //_INCLUDE_BASENETHANDLE_H_