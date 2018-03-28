/********************************************************************
	created:	2011/05/24
	created:	24:5:2011   3:11
	filename: 	h:\2011_02_27\Code\common\NetDll\Net\BaseNetManager.h
	file path:	h:\2011_02_27\Code\common\NetDll\Net
	file base:	BaseNetManager
	file ext:	h
	author:		杨文鸽
	
	purpose:	CS-TCP网络功能接口
*********************************************************************/
#ifndef _INCLUDE_BASENETMANAGER_H_
#define _INCLUDE_BASENETMANAGER_H_

#include "NetHead.h"
#include "EasyList.h"
#include "AutoString.h"
#include "Array.h"
#include "EasyString.h"

#include "NetPlayer.h"
//----------------------------------------------------------------------------------------------


#ifndef BOOL
typedef int		BOOL;
#endif


class tNetObject;
class SocketInputStream;
class SocketOutputStream;
class Packet;
class Socket;
class Player;
class ServerSocket;

class tSafeCheck;
class tNetHandle;

struct fd_set;
struct timeval;

class tNetManager;

class Player;


//----------------------------------------------------------------------------------------------
// 客户端网络

//----------------------------------------------------------------------------------------------

#endif //_INCLUDE_BASENETMANAGER_H_