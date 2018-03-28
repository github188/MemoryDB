
//#include "NetConnect.h"
//#include "NiceDataFieldInfo.h"
//#include "AutoObjectFieldInfo.h"

#include <Windows.h>

namespace FieldInfoName
{
	extern const char fieldNetTypeName[] = "NET_CONNECT";
}


#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	if (DLL_PROCESS_ATTACH==ul_reason_for_call)
	{
		//NiceDataField::getMe().appendDataFieldOnTypeInfo<AConnect>(MEM_NEW AutoObjFieldInfo<AConnect, FIELD_NET_CONNECT, FieldInfoName::fieldNetTypeName>());

	}
	else if (DLL_PROCESS_DETACH==ul_reason_for_call)
	{
		
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif
