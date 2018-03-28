
#include "MeshedNetNodeData.h"
#include "ServerIPInfo.h"

bool NetNodeConnectData::Update( float t )
{
	if (mConnectNodeNetThread==NULL)
		return false;

	if (mConnectNodeNetThread->IsConnectFinish())
	{
		if (mConnectNodeNetThread->mSocket!=0)
		{
			mTryConnectCount = 0;
			HandConnect conn = mMeshedNet->CreateConnect();

			NodeRequestConnect *pRecConnect = dynamic_cast<NodeRequestConnect*>(conn.getPtr());
			AssertEx(pRecConnect!=NULL, "必定为 NodeRequestConnect ");

			mNodeConnect = conn;

			pRecConnect->Init(mMeshedNet, mConnectNodeNetThread->mSocket, mConnectNodeNetThread->mIP.c_str(), mConnectNodeNetThread->mPort);
			//pRecConnect->mServerNetKey = mServerNetKey;
			pRecConnect->mNetNodeConnectData = this;

			mConnectNodeNetThread->mSocket = 0;

			mMeshedNet->_AppendConnect(conn);
			mMeshedNet->_SendSafeCode(pRecConnect);
			pRecConnect->OnConnected();
			mMeshedNet->OnConnected(pRecConnect);
			mMeshedNet->OnAppendNetNode(this);
			mbClient = true;
			Finish();
		}
		else
		{
			Log("NET: WARN Connect fail [%s:%d]", mConnectNodeNetThread->mIP.c_str(), mConnectNodeNetThread->mPort);
			mMeshedNet->OnConnectFail(GetSelf());
		}
		mConnectNodeNetThread->Close();
		delete mConnectNodeNetThread;
		mConnectNodeNetThread = NULL;

	}
	else if (mConnectNodeNetThread->IsOverTime())
	{
		mMeshedNet->OnConnectFail(GetSelf());
		Log("NET: WARN Connect over time [%s:%d]", mConnectNodeNetThread->mIP.c_str(), mConnectNodeNetThread->mPort);
		mConnectNodeNetThread->Close();
		delete mConnectNodeNetThread;
		mConnectNodeNetThread = NULL;

	}
	else
		return true;

	return false;
}

bool NetNodeConnectData::_OnTimeOver()
{
	if (mConnectNodeNetThread!=NULL)
	{
		mConnectNodeNetThread->Close();
		delete mConnectNodeNetThread;
		mConnectNodeNetThread = NULL;
	}		

	if (!IsMainNode() && mMeshedNet->GetTryConnectCount()>=0 && ++mTryConnectCount>mMeshedNet->GetTryConnectCount())
	{
		// 移除
		mMeshedNet->RemoveNode(this);
	}
	else
	{
		mConnectNodeNetThread = MEM_NEW ConnectNetThread();
		int p1, p2 = 0;
		AString strIP = ServerIPInfo::Num2IP(mServerNetKey, p1, p2);
		mConnectNodeNetThread->StartConnect(strIP.c_str(), p1, mMeshedNet->GetConnectOverTime());
		if (IsMainNode())
		{
			NOTE_LOG("启动一次重连主节点　[%s:%d]", strIP.c_str(), p1);
		}
		else
			NOTE_LOG("启动一次重连　[%s:%d]", strIP.c_str(), p1);
		DoEvent(false);
	}

	return true;
}

void NetNodeConnectData::_Free()
{
	if (mConnectNodeNetThread!=NULL)
	{
		mConnectNodeNetThread->Close();
		delete mConnectNodeNetThread;
		mConnectNodeNetThread = NULL;
	}
	if (mNodeConnect)
		mNodeConnect->SetRemove(true);
	mNodeConnect.setNull();
}
