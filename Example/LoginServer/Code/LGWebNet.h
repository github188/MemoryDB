#ifndef _INCLUDE_LGWEBNET_H_
#define _INCLUDE_LGWEBNET_H_

#include "WebGameNet.h"

class LoginThread;
class LGWebNet : public WebGameNet
{
public:
    HandConnect CreateConnect() override;

    bool OnAddConnect(tNetConnect *pConnect);

public:
    LGWebNet(LoginThread *pThread);

public:
    LoginThread *mpLoginThread;
};




#endif //_INCLUDE_LGWEBNET_H_