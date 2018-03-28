

#ifndef _INLCLUDE_GAMESERVEREVENT_H_
#define _INLCLUDE_GAMESERVEREVENT_H_

#include "CEvent.h"
#include "ServerBaseHead.h"
#include "ServerEvent.h"

// 使用网络类型发送
// 1 将网络对象以类型,注册到事件中心
// 2 发送目标以CODE类型进行处理, 第一位为类型,剩下的为连接ID


class SC_NodifyClientConnectOk_S : public Logic::CEvent
{
public:
	virtual bool _DoEvent(void)
	{
		return true;
	}

	void SetConnectID( int id )
	{
		set("ID", id);
	}

protected:

};

class tConnect;

class ServerBase_Dll_Export SC_NodifyClientConnectOk_C : public Logic::tBaseNetEvent
{
public:
	SC_NodifyClientConnectOk_C()
	{

	}

public:
	virtual bool _DoEvent(void);


};

#endif //_INLCLUDE_GAMESERVEREVENT_H_