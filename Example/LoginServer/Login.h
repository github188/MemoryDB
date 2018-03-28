#ifndef _INCLUDE_LOGIN_H_
#define _INCLUDE_LOGIN_H_

#include "BaseServer.h"
#include "LoginThread.h"
#include "AutoString.h"

class Login : public tBaseServer
{
public:
    virtual void ProcessCommand(const char *szCommand)
    {
        //if (AString("a") == AString(szCommand) || AString("A") == AString(szCommand))
        //{
        //    LoginThread* loginThread = (LoginThread*)mBaseThread;
        //    loginThread->UpdateAnnouncementTable();
        //}
    }
};


#endif //_INCLUDE_LOGIN_H_