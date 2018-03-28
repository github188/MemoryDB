#include "IOCPCommon.h"
#include "IOCPServerNet.h"

void IOCPListenThread::Close()
{
	_ForceClose(10);		
	mConnectListLock.lock();

	for (int i=0; i<mConnectList.size(); ++i)
	{
		SocketAPI::closesocket_ex(mConnectList[i].mSocket);
	}

	mConnectList.clear();
	mMainConnectList.clear();
	mConnectListLock.unlock();
	WorkThread::Close();

	if (mListenSocket!=0)
		SocketAPI::closesocket_ex(mListenSocket);
	mListenSocket = INVALID_SOCKET;
}

void IOCPListenThread::OnAcceptConnect(HandConnect newConnect) 
{ 
	mpServerNet->OnAcceptConnect(newConnect); 
}

HandConnect IOCPListenThread::_CreateNewConnect(SocketInfo &socketID) 
{ 
	return mpServerNet->_CreateNewConnect(&socketID); 
}
//-------------------------------------------------------------------------*
NetListenTool::NetListenTool() 
	: mpListen(new NetListen())
{

}

NetListenTool::~NetListenTool()
{
	mpListen->Close();
	SAFE_DELETE(mpListen);
}

void NetListenTool::ProcessToNet( tNetHandle *destHandle )
{
	mpListen->ProcessToNet(destHandle);
}

void NetListenTool::AppendNet( tNetHandle *pNet )
{
	mpListen->AppendNet(pNet);
}

void NetListenTool::RemoveNet( tNetHandle *pNet )
{
	mpListen->RemoveNet(pNet);
}

bool NetListenTool::InitStart( const char *szIp, int port )
{
	return mpListen->InitStart(szIp, port);
}

//-------------------------------------------------------------------------*