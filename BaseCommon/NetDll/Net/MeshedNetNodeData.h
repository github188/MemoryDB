#ifndef _INCLUDE_MESHEDNETNODEDATA_H_
#define _INCLUDE_MESHEDNETNODEDATA_H_

#include "NetHead.h"
#include "CEvent.h"
#include "NetHandle.h"
#include "IOCPCommon.h"
#include "MeshedNodeNet.h"

#include "AnyData.h"

// 连接到其他节点的连接事件, 用于重连处理
class Net_Export NetNodeConnectData : public Logic::CEvent
{
public:
	NetNodeConnectData()
		: mConnectNodeNetThread(NULL)
		, mbClient(false)
		, mTryConnectCount(0)
	{

	}
	~NetNodeConnectData()
	{
		_Free();
	}

public:
	virtual bool _DoEvent()
	{
		setFinished(false);
		WaitTime((float)mMeshedNet->GetConnectOverTime());
		StartUpdate(0.5f);

		return true;
	}

	virtual bool Update(float t);

	virtual bool _OnTimeOver();

	virtual void _Free();

	virtual bool IsConnected(){ return mNodeConnect && !mNodeConnect->IsDisconnect(); }

	virtual void InitData() override
	{
		Logic::CEvent::InitData();
		_Free();
		mTryConnectCount = 0;	
	}

	virtual bool IsMainNode() const
	{
		return mServerNetKey==mMeshedNet->mMainNodeKey;
	}

	void SetUseData(AutoAny any){ mUseData = any; }
	AutoAny GetUseData() { return mUseData; }

public:
	HandConnect			mNodeConnect;
	ConnectNetThread	*mConnectNodeNetThread;

	Hand<MeshedNodeNet>	mMeshedNet;
	AutoAny				mUseData;

	GSKEY				mServerNetKey;
	int					mTryConnectCount;
	bool				mbClient;
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_MESHEDNETNODEDATA_H_