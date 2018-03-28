
#ifndef _INCLUDE_SCRIPTFACE_H_
#define _INCLUDE_SCRIPTFACE_H_

#include "Event.h"
#include "AutoString.h"

namespace LuaPlus
{
	class LuaState;
	class LuaObject;
}

namespace Logic
{
	class tScriptFace
	{
	public:
		virtual ~tScriptFace() {}

		virtual bool ExeFunction(AutoEvent &event, const char* functionName, void *eventObj) = 0;
		virtual bool ExeString(AutoEvent &event, const char* scriptString) = 0;

		virtual bool ExeFunction(AutoEvent &event, const char* functionName, void *triggerEvent, 
			const char* actionName) = 0;

		virtual LuaPlus::LuaState* GetLuaState(void) = 0;
	};

}


#endif //_INCLUDE_SCRIPTFACE_H_