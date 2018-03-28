#include "BaseCommon.h"

#include "NetConfig.h"

#ifndef Net_Export
#ifdef STATE_NET_LIB
#define Net_Export
#define Net_Export_H	
#else
#		ifdef NET_EXPORTS
#			define Net_Export __declspec(dllexport)
#			define Net_Export_H	__declspec(dllexport)
#		else
#			define Net_Export __declspec(dllimport)
#			define Net_Export_H	
#		endif
#endif

typedef unsigned long	IP_t;

#define IP_SIZE				(24)
#define NET_CONNECT_MAX		(10280)
#define	MAX_UDP_MSG_SIZE	(1400)		//1472 - 9(packet code and index)

#ifndef FD_SETSIZE
#define FD_SETSIZE			(NET_CONNECT_MAX)
#endif


enum NET_CONNECT_STATE
{
	NET_NO_USE,
	NET_INIT_CONNECT,
	NET_CONNECT_BEGIN,	
	NET_CONNECT_PASS,
	NET_CONNECT_OK,
};

enum RECEIVE_RESULT
{
	RECEIVE_FAIL,
	RECEIVE_SUCCEED,
	RECEIVE_SUCCEED_FINISH,
};

#endif

#include "NetConfig.h"



