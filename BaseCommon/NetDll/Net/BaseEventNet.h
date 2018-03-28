
#ifndef _INCLUDE_BASEEVENTNET_H_
#define _INCLUDE_BASEEVENTNET_H_

#include "NetHandle.h"
#include "EventProtocol.h"
#include "NetHead.h"
//----------------------------------------------------------------------------------------------


class Net_Export  tBaseEventNet : public tNetHandle
{
public:
	tBaseEventNet();
	virtual ~tBaseEventNet();

public:
	virtual void SetIndexName(const char* strIndexName) { mIndexName = strIndexName; }
	virtual const char*  GetIndexName(void) const { return mIndexName.c_str(); }

public:
	//virtual bool StartNet(void) = 0;
	virtual bool StartNet(const char *szIP, int port){ return false; }

	// Define over time 10 second.
	virtual int GetConnectOverTime(void) { return 10; } 
	virtual int GetTryConnectCount(void) { return 6; }
	virtual const char* GetConnectFinishEvent(void) { return NULL; }

	// 服务器才需要处理这个加入连接功能
	virtual bool OnAddConnect(tNetConnect *pConnect){ return true; }
	virtual void OnCloseConnect(tNetConnect *pConnect){}
	virtual bool IsOk(void){return true;}
	virtual bool TryReady(void){ return true;}

public:	
	virtual bool OnReceiveEvent(tNetConnect *revConnect, Logic::tEvent *revEvent);	

public:
	virtual void RegisterEvent(AutoEventCenter hCenter){}
	virtual void BindEventCenter()
	{
		GetEventCenter()->_SetNetTool(0, GetSelf());
	}

public:
	virtual void SetNetProtocol(Auto<tNetProtocol> netProtocol)
	{
		if (mNetProtocol)
			netProtocol->AppendNetPacketFrom(mNetProtocol.getPtr(), false);
		mNetProtocol = netProtocol; 
	}
	virtual tNetProtocol* GetNetProtocol(void) { return mNetProtocol.getPtr(); }

	// 发送和接收, 默认不进行任何转换, 如果对方字符集不一至, 则可以在下面进行转换
	virtual AString OnSendStringConvertCode(const char *szSendString){ return AString(); }
	virtual AString OnReceiveStringConverCode(const char *szReceiveString){		return AString(); }	

	virtual void Log(const char *szInfo, ...);

protected:
	EasyString					mIndexName;
	Auto<tNetProtocol>		mNetProtocol;
};


#endif //_INCLUDE_BASEEVENTNET_H_