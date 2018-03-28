
#include "BaseServer.h"
#include "ShareMemAO.h"
//-------------------------------------------------------------------------
#include "TableManager.h"
//#include "EventCenterManager.h"


#include "BaseThread.h"

#ifdef __WINDOWS__
#include <Windows.h>
#endif
//#include "UDPEasyNet.h"

tBaseServer::tBaseServer() 
	: mBaseThread(NULL)	
	, mAlready(false)
{
	new TableManager();
}


tBaseServer::~tBaseServer()
{
	//if (TableManager::getSingletonPtr())
	//delete TableManager::getSingletonPtr();
}


bool tBaseServer::InitConfig( const char* szConfigTableName )
{
	if (szConfigTableName==NULL || strcmp(szConfigTableName, "")==0)
		return false;
	TableManager::SetDefaultPath("./");
	return TableManager::getSingleton().LoadTable(szConfigTableName, "CSV");
}



bool tBaseServer::InitStaticData(BaseThread	 *pBaseThread)
{
	ReleaseStaticData();

	mBaseThread = pBaseThread;
	mBaseThread->InitEventCenter(AutoEventCenter());

	mAlready = true;
	return true;
}

void tBaseServer::ReleaseStaticData(void)
{
	if (!mAlready)
		return;

	mBaseThread->Close();
	SAFE_DELETE(mBaseThread);

	delete TableManager::getSingletonPtr();

	mAlready = false;
	return;
}



bool tBaseServer::Start(BaseThread *pBaseThread, const char *szProcessName, const char* szLogFileName, const char* szConfigTableName)
{
	Allot::setLogFile(szProcessName);
	TableManager::SetLog( new TableLog(szLogFileName) );

	InitConfig(szConfigTableName);
	
	InitStaticData(pBaseThread);

	mBaseThread->start();

	return true;
}


void tBaseServer::RequestClose()
{
	if (mBaseThread!=NULL)
		mBaseThread->mQuestClose = true;
}

void tBaseServer::Stop(void)
{
	mBaseThread->Close();
	ReleaseStaticData();
}

bool tBaseServer::IsStop()
{
	if (mBaseThread!=NULL) return mBaseThread->IsStop(); return false;
}

#ifdef __WINDOWS__

bool tBaseServer::SetIocn(const char *szIconBmpFile, const char *szText, const char *szFontName, int nFontSize, unsigned int color)
{
	if (szIconBmpFile==NULL)
		return false;

	HMODULE m = GetModuleHandle(NULL);
	HANDLE hbitmap = LoadImage(NULL,szIconBmpFile,IMAGE_BITMAP,0,0,LR_LOADFROMFILE); 

	if (hbitmap==(HANDLE)INVALID_HANDLE || hbitmap==NULL)
		return false;

	HDC hMemDC = NULL;
	HFONT f = NULL;
	if (szText!=NULL)
	{	
		if (szFontName!=NULL)
			f = CreateFont(nFontSize,0,0,0,100,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_SWISS, szFontName);

		HDC hDC = ::GetDC(GetConsoleWindow());
		//CFont * pOldFont = hDC->SelectObject();
		hMemDC = CreateCompatibleDC(hDC);
		if (f!=NULL)
			SelectObject(hMemDC, f);
		SelectObject(hMemDC, hbitmap);
		//在位图上写字
		RECT rect = {0, 0, 600, 600};
		SetTextColor(hMemDC, RGB(color>>16 & 0xFF, color>>8 & 0xFF, color&0xFF));
		SetBkMode(hMemDC, TRANSPARENT);
		DrawText(hMemDC, szText, -1, &rect, DT_VCENTER);
	}
	BITMAP bmp;  
	GetObject(hbitmap,sizeof(BITMAP),&bmp);

	HBITMAP hbmMask = ::CreateCompatibleBitmap(::GetDC(NULL), 
		bmp.bmWidth, bmp.bmHeight);

	ICONINFO ii = {0};
	ii.fIcon = TRUE;
	ii.hbmColor = (HBITMAP)hbitmap;
	ii.hbmMask = hbmMask;

	HICON hIcon = ::CreateIconIndirect(&ii);//一旦不再需要，注意用DestroyIcon函数释放占用的内存及资源
	if (hMemDC!=NULL)
		DeleteDC(hMemDC);
	if (f!=NULL)
		DeleteObject(f);
	::DeleteObject(hbmMask);

	HWND   hwnd=GetConsoleWindow();//直接获得前景窗口的句柄   

	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);   
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	return true;
}
#else
bool tBaseServer::SetIocn(const char *szIconBmpFile, const char *szText, const char *szFontName, int nFontSize, unsigned int color)
{
	return false;
}
#endif

//-------------------------------------------------------------------------
tProcess::tProcess() 
	: mShareMem(NULL)
	, mbReadyShareMemOk(false)
{

}

tProcess::~tProcess()
{
	if (mShareMem)
		delete mShareMem;
	mShareMem = NULL;
}

bool tProcess::InitShareMem( unsigned long memKey, unsigned int uSize, bool bCreate )
{
	if (mShareMem==NULL)
		mShareMem = new ShareMemAO();

	if (bCreate)
		mbReadyShareMemOk = mShareMem->Create(memKey, uSize)==TRUE;
	else
		mbReadyShareMemOk = mShareMem->Attach(memKey, uSize)==TRUE;

	return mbReadyShareMemOk;
}

bool tProcess::Process()
{
	// 检查共享内存消息
	if (mbReadyShareMemOk && mShareMem!=NULL)
	{
		char *pMsg = mShareMem->GetDataPtr();
		if (pMsg!=NULL)
			return OnShareMemMsg(pMsg);		
	}
	return true;
}

char* tProcess::GetShareData()
{
	if (mShareMem!=NULL)
		return mShareMem->GetDataPtr();
	return NULL;
}
