/********************************************************************
	created:	2011/05/31
	created:	31:5:2011   3:01
	filename: 	e:\EventSystem\EventCore\Core\DllFace.h
	file path:	e:\EventSystem\EventCore\Core
	file base:	DllFace
	file ext:	h
	author:		杨文鸽
	
	purpose:	
*********************************************************************/
#ifndef _INCLULD_DLLFACE_H_
#define _INCLULD_DLLFACE_H_

#include "EventCore.h"
#include "EasyList.h"
#include "NetHandle.h"

namespace Logic
{
	class tEventCenter;
	struct PluginInfo;
	struct PLUGIN_INFO
	{
		PluginInfo	*pluginInfo;

		PLUGIN_INFO():pluginInfo(NULL){}
		PLUGIN_INFO( PluginInfo *info )
			: pluginInfo(info) 
		{
		}
	};

	class EventCoreDll_Export tEventDllPlugin
	{
	public:
		tEventDllPlugin(tEventCenter *eventCenter);
		virtual ~tEventDllPlugin();

	public:
		bool LoadPluginDll( const char* szPluginName );
	
		void FreeAllDll( void );

	protected:
		//所有插件
		EasyList< PluginInfo* >		mPluginList;

		tEventCenter				*mEventCenter;
	};

	struct NetPluginInfo;

	// 用于加载网络DLL
	class EventCoreDll_Export NetDllPlugin	: public MemBase
	{
	public:
		NetDllPlugin();
		~NetDllPlugin();
	public:
		bool LoadPluginDll( const char* szPluginName );
		void FreeNetDll();

	public:
		bool InitNet(tNetHandle *pServerTool);

	protected:
		NetPluginInfo	*mPluginInfo;
	};
}

#endif //_INCLULD_DLLFACE_H_