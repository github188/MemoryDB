#ifndef _INCLUDE_EVENTDLLFACE_H_
#define _INCLUDE_EVENTDLLFACE_H_

#include "EventFactory.h"
#include "EventCenter.h"

namespace Logic
{

	class tEventDllFace
	{
	public:
		virtual ~tEventDllFace()
		{
			// 这里进行强行释放,如果不这样,在现在之后,使用到的模块创建的工厂都会出错
			// 这模块内创建的对象,会在模块关闭后全部收回
			for ( size_t i=0; i<mFactoryList.size(); ++i )
			{
				mFactoryList[i]._free();
			}
		}

		void AppendFacotry( tEventCenter *eventCenter, const char* eventName, tEventFactory *pFactory )
		{
			AutoEventFactory hFact = pFactory;
			pFactory->__SetSelfHandle(hFact);
			mFactoryList.push_back(hFact);
			eventCenter->RegisterEvent(eventName, hFact);
		}

	protected:
		Array<AutoEventFactory>	mFactoryList;
	};


}

#endif //_INCLUDE_EVENTDLLFACE_H_