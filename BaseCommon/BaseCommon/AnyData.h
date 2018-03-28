
#ifndef _INCLUDE_ANYDATA_H_
#define _INCLUDE_ANYDATA_H_

#include "Hand.h"

class AnyData : public Base<AnyData>
{
public:
	~AnyData(){}
};

typedef Hand<AnyData> AutoAny;


#endif //_INCLUDE_ANYDATA_H_