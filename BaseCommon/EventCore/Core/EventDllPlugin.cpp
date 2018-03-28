/********************************************************************
	created:	2011/05/31
	created:	31:5:2011   2:58
	filename: 	e:\EventSystem\EventCore\Core\DllFace.cpp
	file path:	e:\EventSystem\EventCore\Core
	file base:	DllFace
	file ext:	cpp
	author:		杨文鸽
	
	purpose:	
*********************************************************************/

#include "EventDllPlugin.h"
#include <windows.h>
#include "AutoString.h"
#include "NetHandle.h"
namespace Logic
{
	class tEventFactory;

	typedef void (__cdecl * FUNC_DLLINIT)( tEventCenter * );
	//插件必须导出的函数DllRelease
	typedef void (__cdecl * FUNC_DLLRELEASE)(tEventFactory *);
	//插件信息
	struct PluginInfo : MemBase
	{
		HMODULE			hModule;
		AString			strPluginFile;
		FUNC_DLLINIT	pfnDllInit;
		FUNC_DLLRELEASE pfnDllRelease;
	};



	tEventDllPlugin::tEventDllPlugin(tEventCenter *eventCenter)
		: mEventCenter(eventCenter)
	{

	}

	tEventDllPlugin::~tEventDllPlugin()
	{
		FreeAllDll();
	}

	bool tEventDllPlugin::LoadPluginDll( const char* szPluginName )
	{
		if(NULL==szPluginName || NULL==mEventCenter) return false;

		HMODULE hModule = NULL;
		FUNC_DLLINIT pfnDllInit = NULL;
		FUNC_DLLRELEASE pfnDllRelease = NULL;

		try
		{
			//尝试Load该dll
			hModule = ::LoadLibrary(szPluginName);
			if(!hModule)
			{
				DWORD dLastError = ::GetLastError();
				//ERROR_LOG(("Load plugin dll(%s) failed!"), szPluginName);
				return false;
			}

			//查看是否有合法的导出函数
			pfnDllInit = (FUNC_DLLINIT)::GetProcAddress(hModule, "AppendEventFactory");
			pfnDllRelease = (FUNC_DLLRELEASE)::GetProcAddress(hModule, "FreeEventFactory");
			if(NULL==pfnDllInit || NULL==pfnDllRelease)
			{ 
				::FreeLibrary(hModule); 
				hModule = NULL; 
				//ERROR_LOG(("Wrong plugin dll(%s) export function 'getAllPacketFactory'!"), szPluginName);
				return false; 
			}
			(*pfnDllInit)( mEventCenter );
		}
		catch(...) 
		{
			if(hModule)
				::FreeLibrary(hModule);
			return false;
		}

		PluginInfo *newPlugin = MEM_NEW PluginInfo();
		newPlugin->strPluginFile = szPluginName;
		newPlugin->hModule = hModule;
		newPlugin->pfnDllInit = pfnDllInit;
		newPlugin->pfnDllRelease = pfnDllRelease;

		mPluginList.push_back(newPlugin);
		return true;
	}

	void tEventDllPlugin::FreeAllDll( void )
	{
		//释放所有插件
		for(EasyList< PluginInfo* >::iterator it = mPluginList.begin(); it!=mPluginList.end(); ++it)
		{
			PluginInfo *pluginInfo = (*it);
			(*(pluginInfo->pfnDllRelease))(NULL);
			::FreeLibrary(pluginInfo->hModule);
			pluginInfo->hModule = NULL;
			delete pluginInfo;
		}
		mPluginList.clear();
	}

	//-------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------

	typedef AutoNet (__cdecl * FUNC_NETDLL_INIT)( tNetHandle* );
	//插件必须导出的函数DllRelease
	typedef void (__cdecl * FUNC_NETDLL_RELEASE)(AutoNet );
	//插件信息
	struct NetPluginInfo : MemBase
	{
		HMODULE				hModule;
		AString				strPluginFile;
		FUNC_NETDLL_INIT	pfnDllInit;
		FUNC_NETDLL_RELEASE pfnDllRelease;
	};
	//-------------------------------------------------------------------------------------------------
	NetDllPlugin::NetDllPlugin()
		: mPluginInfo(NULL)
	{

	}
	NetDllPlugin::~NetDllPlugin()
	{
		FreeNetDll();
	}

	bool NetDllPlugin::LoadPluginDll( const char* szPluginName )
	{
		FreeNetDll();
		if(NULL==szPluginName) return false;

		HMODULE hModule = NULL;
		FUNC_NETDLL_INIT pfnDllInit = NULL;
		FUNC_NETDLL_RELEASE pfnDllRelease = NULL;

		try
		{
			//尝试Load该dll
			hModule = ::LoadLibrary(szPluginName);
			if(!hModule)
			{
				DWORD dLastError = ::GetLastError();
				//ERROR_LOG(("Load plugin dll(%s) failed!"), szPluginName);
				return false;
			}

			//查看是否有合法的导出函数
			pfnDllInit = (FUNC_NETDLL_INIT)::GetProcAddress(hModule, "CreateNet");
			pfnDllRelease = (FUNC_NETDLL_RELEASE)::GetProcAddress(hModule, "ReleaseNet");
			if(NULL==pfnDllInit || NULL==pfnDllRelease)
			{ 
				::FreeLibrary(hModule); 
				hModule = NULL; 
				//ERROR_LOG(("Wrong plugin dll(%s) export function 'getAllPacketFactory'!"), szPluginName);
				return false; 
			}
		}
		catch(...) 
		{
			if(hModule)
				::FreeLibrary(hModule);
			return false;
		}

		mPluginInfo = MEM_NEW NetPluginInfo();
		mPluginInfo->strPluginFile = szPluginName;
		mPluginInfo->hModule = hModule;
		mPluginInfo->pfnDllInit = pfnDllInit;
		mPluginInfo->pfnDllRelease = pfnDllRelease;
		return true;
	}

	void NetDllPlugin::FreeNetDll()
	{
		if (mPluginInfo)
		{
			//if (mNetTool)
			//{
			//	mNetTool._free();
			//	//mPluginInfo->pfnDllRelease(mNetTool);
			//	//mNetTool = NULL;
			//}
			::FreeLibrary(mPluginInfo->hModule);
			delete mPluginInfo;
			mPluginInfo = NULL;
		}
	}

	bool NetDllPlugin::InitNet( tNetHandle *pServerTool )
	{		
		if (NULL==pServerTool)
			return false;
		mPluginInfo->pfnDllInit(pServerTool);
		return  true;
	}

}