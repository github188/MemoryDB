#ifndef _INCLUDE_RECORDTOOL_H_
#define _INCLUDE_RECORDTOOL_H_

#include "ArrayList.h"

template <typename T>
class SaveDBArray : public ArrayList<T>
{
public:
	Auto<BaseRecord> re;
	int nCol;

	SaveDBArray()
		: nCol(-1) {}

	~SaveDBArray()
	{
		if (re && nCol >= 0)
		{
			AutoData data = re->getDataBuffer(nCol);
			if (!data)
				data = MEM_NEW DataBuffer(64);
			data->seek(0);
			data->writeArray(*this);
			re->set(nCol, &data, typeid(data));
		}
	}
};

#endif //_INCLUDE_RECORDTOOL_H_

