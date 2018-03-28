//#ifndef _INCLUDE_SERVERNETTHREAD_H_
//#define _INCLUDE_SERVERNETTHREAD_H_
//
//#include "NetHead.h"
//#include "BackThreadMgr.h"
//#include "BackTask.h"
//
//class Socket;
//class ServerSocket;
//
//class ConnectTask : public BackTask
//{
//public:
//	ConnectTask();
//	~ConnectTask();
//	virtual bool autoFinish(void) const { return true; }
//	virtual bool removeOnFinish() const { return true; }
//
//	virtual void ExeBack(BackThreadMgr *mgr) {}
//	virtual void ExeFinished() {}
//
//	virtual bool Lock() { return true; }
//	virtual bool UnLock() { return true; }
//	virtual bool TryLock() { return true; }
//
//public:
//	Socket		*mSocket;
//};
//
//class Net_Export ServerNetThread : public BackThreadMgr
//{
//public:
//	ServerNetThread();
//
//	void InitServerSocket( unsigned int port );
//
//	virtual void backWorkTread(void);
//
//	virtual void Process( void );
//
//	virtual void OnConnect( Socket *socket ) = 0;
//
//protected:
//	ServerSocket	*mServerSocket;
//};
//
//#endif //_INCLUDE_SERVERNETTHREAD_H_