
#ifndef _DBTRIGGER_H_
#define _DBTRIGGER_H_

#include "BaseRecord.h"

class DBTrigger : public Base<DBTrigger>
{
public:
	virtual void OnTrigger(AutoRecord record) = 0;
	virtual bool NeedTrigger(int nCol) = 0;
};

typedef Hand<DBTrigger>	HandTrigger;

#endif //_DBTRIGGER_H_