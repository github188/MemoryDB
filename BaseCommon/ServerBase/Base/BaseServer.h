
#ifndef _INCLUDE_BASESERVER_H_
#define _INCLUDE_BASESERVER_H_

#include "ServerBaseHead.h"

class BaseThread;
//-------------------------------------------------------------------------
class ServerBase_Dll_Export tBaseServer
{
public:
	tBaseServer();
	virtual ~tBaseServer();

public:

	virtual bool InitConfig(const char* szConfigTableName);
	virtual bool InitStaticData(BaseThread *pBaseThread);
	virtual void ReleaseStaticData(void);
	virtual bool Start(BaseThread *pBaseThread, const char *szProcessName, const char* szLogFileName, const char* szConfigTableName);

	virtual void RequestClose();
	virtual void Stop(void);
	virtual bool IsStop();
	virtual bool CanStop(){ return true; }

	virtual void ProcessCommand(const char *szCommand){}

	// 128 * 128 nFontSize = 100
	static bool SetIocn(const char *szIconBmpFile, const char *szText, const char *szFontName, int nFontSize, unsigned int color);

protected:
	BaseThread				*mBaseThread;
	bool							mAlready;
};


//-------------------------------------------------------------------------
class ShareMemAO;

class ServerBase_Dll_Export tProcess
{
public:
	tProcess();
	virtual ~tProcess();

public:
	virtual bool OnShareMemMsg(char *) = 0;

public:
	virtual bool InitShareMem(unsigned long memKey, unsigned int uSize, bool bCreate);
	virtual char* GetShareData();
	virtual bool Process();

protected:
	ShareMemAO						*mShareMem;			// 用于进程控制
	bool							mbReadyShareMemOk;
};




#endif //_INCLUDE_BASESERVER_H_
