/********************************************************************
	created:	2011/07/21
	created:	21:7:2011   0:12
	filename: 	d:\Work\Common\DataBase\StringTool.h
	file path:	d:\Work\Common\DataBase
	file base:	StringPool
	file ext:	h
	author:		ÑîÎÄ¸ë
	
	purpose:	
*********************************************************************/
#ifndef _INCLUDE_STRINGPOOL_H_
#define _INCLUDE_STRINGPOOL_H_

#include "MemBase.h"
#include "BaseCommon.h"

//------------------------------------------------------------------------------------
#define STRING(v)				StringTool(v).c_str()
#define TOINT(sz)				StringTool::Int(sz)
#define TOFLOAT(sz)				StringTool::Float(sz)
#define TOINT64(sz)				((Int64)StringTool::_UInt64(sz))
#define TOUINT64(sz)			StringTool::_UInt64(sz)

#define TEMP_CHAR_LENGTH		(32)
//------------------------------------------------------------------------------------

class StringTool
{
public:
	operator const char* () const
	{
		return mszTemp;
	}

	const char* c_str() const
	{
		return mszTemp;
	}

	StringTool(int x)
	{
		_itoa_s(x, mszTemp, TEMP_CHAR_LENGTH, 10);
		//return mszTemp;
	}

	StringTool(float x)
	{
		sprintf_s(mszTemp, TEMP_CHAR_LENGTH, "%.7f", x);
		//return mszTemp;
	}

	StringTool(double x)
	{
		sprintf_s(mszTemp, TEMP_CHAR_LENGTH, "%lf", x);
		//return mszTemp;
	}

	StringTool(bool x)
	{
		_itoa_s( (x?1:0), mszTemp, TEMP_CHAR_LENGTH, 10);
		//return mszTemp;
	}

	StringTool(UInt64 x)
	{
		sprintf_s(mszTemp, TEMP_CHAR_LENGTH, "%llu", x);
		//return mszTemp;
	}

	StringTool(Int64 x)
	{
		_i64toa_s(x, mszTemp, TEMP_CHAR_LENGTH, 10);
		//return mszTemp;
	}

	StringTool()
	{
		memset(mszTemp, 0, TEMP_CHAR_LENGTH);
	}

public:
	const char* ToString(int x)
	{
		_itoa_s(x, mszTemp, TEMP_CHAR_LENGTH, 10);
		return mszTemp;
	}

	const char* ToString(float x)
	{
		sprintf_s(mszTemp, TEMP_CHAR_LENGTH, "%.7f", x);
		return mszTemp;
	}

	const char* ToString(double x)
	{
		sprintf_s(mszTemp, TEMP_CHAR_LENGTH, "%lf", x);
		return mszTemp;
	}

	const char* ToString(bool x)
	{
		_itoa_s( (x?1:0), mszTemp, TEMP_CHAR_LENGTH, 10);
		return mszTemp;
	}

	const char* ToString(UInt64 x)
	{
		sprintf_s(mszTemp, TEMP_CHAR_LENGTH, "%llu", x);
		return mszTemp;
	}

	const char* ToString(Int64 x)
	{
		_i64toa_s(x, mszTemp, TEMP_CHAR_LENGTH, 10);
		return mszTemp;
	}

	static int Int(const char *szX)
	{
		return atoi(szX);
	}

	static float Float(const char *szX)
	{
		return (float)atof(szX);
	}

	static double Double(const char *szX)
	{
		return strtod(szX, NULL);
	}

	static UInt64 _UInt64(const char *szX)
	{
		UInt64 x = 0;
		if (sscanf(szX, "%llu", &x) != 1)
			x = 0;

		return x;
	};

	//NOTE: No used because get max int64 value when over
	//static Int64 _Int64(const char *szX)
	//{
	//	return _atoi64(szX);
	//};

public:
	char			mszTemp[TEMP_CHAR_LENGTH];
};
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------



#endif