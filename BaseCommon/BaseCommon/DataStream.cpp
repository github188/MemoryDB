
#include "DataStream.h"

void DataStream::Release(void)
{
	delete this;
} 

bool DataStream::readText( AString &resultString )
{
	DSIZE nowPos = tell();

	int strLen = 0;
	while (true)
	{
		char t = '\0';
		if (!_read((void*)t, 1))
			return false;

		if (t=='\0')
			break;

		++strLen;
	}
	if (strLen>0)
	{
		resultString.Alloc(strLen+1);
		seek(nowPos);
		return _read(resultString._str(), strLen+1);
	}
	return true;
}

bool DataStream::readLine(AString &resultString)
{
	if (empty())
		return false;
	char r[2];
	r[0] = 0;
	r[1] = '\0';
	char temp = 0;
	while (read(r[0]))
	{
		if (r[0]=='\r')
		{
			if (read(temp))
			{
				if (temp=='\n')
					break;
			}
		}
		else if (r[0]=='\n')
			break;

		resultString += r;
	}
	return true;
}

bool DataStream::writeStringArray(const StringArray &scrArray )
{
	write((DSIZE)scrArray.size());
	for (size_t i=0; i<scrArray.size(); ++i)
	{
		writeString(scrArray[i]);
	}
	return true;
}

bool DataStream::readStringArray( StringArray &resultArray )
{
	DSIZE count = 0;
	READ(this, count);
	resultArray.resize(count);
	for (size_t i=0; i<resultArray.size(); ++i)
	{
		if (!readString(resultArray[i]))
			return false;
	}
	return true;
}

