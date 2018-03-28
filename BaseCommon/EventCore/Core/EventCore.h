#include "BaseCommon.h"
//-------------------------------------------------------------------------
#ifndef EventCoreDll_Export
#	ifdef STATE_EVENTCORE_LIB
#		define EventCoreDll_Export
#	else
#		ifdef EVENTCORE_EXPORTS
#			define EventCoreDll_Export __declspec(dllexport)
#		else
#			define EventCoreDll_Export __declspec(dllimport)
#		endif
#	endif
#endif

//-------------------------------------------------------------------------
#ifndef _INCLUDE_EVENTCORE_H_
#define _INCLUDE_EVENTCORE_H_


#define SERIALIZE_EVENT_NAME	0
#define FREE_EVENT_MAX_COUNT	(100)		// �����ͷŵ����¼��������, ������������ֱ���ͷ��¼�

#ifdef __LINUX__
	typedef unsigned long	EVENT_ID;
#else
	typedef unsigned int	EVENT_ID;
#endif


#endif //_INCLUDE_EVENTCORE_H_
//-------------------------------------------------------------------------
