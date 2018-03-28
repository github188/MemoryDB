

#ifndef _INLCLUDE_GAMESERVEREVENT_H_
#define _INLCLUDE_GAMESERVEREVENT_H_

#include "CEvent.h"
#include "ServerBaseHead.h"
#include "ServerEvent.h"

// ʹ���������ͷ���
// 1 ���������������,ע�ᵽ�¼�����
// 2 ����Ŀ����CODE���ͽ��д���, ��һλΪ����,ʣ�µ�Ϊ����ID


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