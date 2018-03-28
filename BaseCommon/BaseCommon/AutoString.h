/********************************************************************
created:	2011/07/21
created:	21:7:2011   0:12
filename: 	d:\Work\Common\DataBase\MemString.h
file path:	d:\Work\Common\DataBase
file base:	MemString
file ext:	h
author:		杨文鸽

purpose:	
*********************************************************************/
#ifndef _INCLUDE_AUTOSTRING_H_
#define _INCLUDE_AUTOSTRING_H_

#include "BaseCommon.h"
#include "AutoPtr.h"
#include "StringTool.h"

#include "Assertx.h"
#include "Array.h"

//-------------------------------------------------------------------------*/
#define TOUTF8(sz)		AString::getUTF8(sz).c_str()
#define TOANIS(sz)		AString::getANIS(sz).c_str()
//-------------------------------------------------------------------------*/
#if DEBUT_STRING
#	include <set>
#endif
//----------------------------------------------------------------------
#define		SIZE_SPACE_LENGTH	sizeof(unsigned short)
//----------------------------------------------------------------------
#ifndef __LINUX__

template<>
void AutoPtr<wchar_t>::FreeClass()
{
	delete[] getPtr();
}

#endif

typedef AutoPtr<wchar_t>		WString;
//------------------------------------------------------------------------------------
// 使用字符缓存保存字符串信息及字符串内容
// mStringBuffer 
// 1 如果最后为0，表示直接保存字符
// 2 最后为'P', 表示保存字符串分配的空间，8字节后保存分配空间可以保存的字符长度, 新的空间开头两字节为引用计数,之后为保存的字符串内容
//------------------------------------------------------------------------------------

#define  StringSpaceInfoSize  (sizeof(UseType))


// 当 DEBUT_STRING =0 >分配总长度为 15
// 优化AData时，使用AData 占用 17个字节，第一个字节为字段类型，使用 EasyMap KeyValue占用32个字节
#define STRING_BUFFER_LENGTH		15	// 分配 STRING_BUFFER_LENGTH+1 字节空间
#define IS_USE_STRING_PTR			(mStrBuffer[STRING_BUFFER_LENGTH]==('P'))


class BaseCommon_Export AString
{
public:
	void Format(const char* pFormat, ...);
	void Format(va_list &valist, const char *pFormat);

	const char* c_str() const
	{
		if (IS_USE_STRING_PTR)
			return _strBuffer_const()+StringSpaceInfoSize;

		return mStrBuffer;
	}

	size_t length() const
	{
		return strlen(c_str());
	}

	bool empty()const
	{
		if (IS_USE_STRING_PTR)
			return NULL==_strBuffer_const();
		return mStrBuffer[0] == '\0';
	}	

	void swap(AString &other)
	{
		AString t = *this;
		*this = other;
		other = t;
	}

	void set(const char *szStr)
	{
		_set(szStr);
	}

	void replace(const char *szScrStr, const char *szDest);

	void replaceFirst(const char *szScrStr, const char *szDest);

	void replaceLast(const char *szScrStr, const char *szDest);

	void free()
	{
		setNull();
	}

public:
	AString operator + (const char *szString);
	AString operator + (const AString &str);
	AString operator + (const wchar_t *wString);

	AString operator + (int nVal);
	AString operator + (float fVal);
	AString& operator += (const char *szString)
	{
		return link(szString);
	}

	AString& operator += (const AString &str)
	{
		return link(str.c_str());
	}

	AString& operator += (const wchar_t *wString);

	AString& operator += (int nVal);
	AString& operator += (float fVal);
	AString& operator += ( double dVal );
	AString& operator += (UInt64 uValU64);

	void linkInt64(BIGINT i64);

	bool getU64(UInt64  &uResult);
	UInt64 getU64(void){ UInt64 v; if (!getU64(v)) v = 0;  return v; }

	bool operator < (const AString &other) const
	{
		return strcmp(c_str(),other.c_str())<0;
	}
	bool operator > (const AString &other) const
	{
		return strcmp(c_str(), other.c_str())>0;
	}
	bool operator == (const AString &other) const
	{
		return strcmp(c_str(),other.c_str())==0;
	}	
	bool operator != (const AString &other) const
	{
		return ! (*this == other);
	}

	bool operator < (const char *szString) const
	{
		return strcmp(c_str(), szString)<0;
	}
	bool operator > (const char *szString) const
	{
		return strcmp(c_str(), szString)>0;
	}
	bool operator == (const char* szString) const
	{
		return strcmp(c_str(), szString)==0;
	}
	bool operator != (const char* szString) const
	{
		return strcmp(c_str(), szString)!=0;
	}

	AString& link(const char *szString);
	WString getWString(uint pagecode = 0) const;

	void toLower();
	const char* toUTF8();
	const char* toANIS();

	int getKeyID() const ;

	int replace(char scrChar, char destChar);

	static AString getUTF8(const char *scr, int pageCode = 0);

	static AString getANIS( const char *szUTF8 );

	static int Split(const char *scrString, Array<AString> &resultStringList, const char *delims, int maxSplits);

	AString SplitLeft(const char *splitString);

	// 获取 {  } 之间的文本, 支持嵌套如 { {a} {b} } 返回 "{a} {b}" 
	AString SplitBlock(const char s1= '{', const char s2 = '}', bool bSkipSplitChar = false);

	void set(const char *sz, DSIZE len);

	void setNull()
	{
		_setnull();
	}

public:
	~AString()
	{
		_setnull();
	}

	AString()
	{
		memset(mStrBuffer, 0, STRING_BUFFER_LENGTH+1);
	}
	AString(const AString &other) 
	{
		memset(mStrBuffer, 0, STRING_BUFFER_LENGTH+1);
		*this = other;
	}
	AString( const char* szString ) 
	{
		memset(mStrBuffer, 0, STRING_BUFFER_LENGTH+1);
		*this = szString;
	}

	AString( const wchar_t* wString ) 
	{
		memset(mStrBuffer, 0, STRING_BUFFER_LENGTH+1);
		*this = wString;
	}

	AString& operator = (const AString &other)
	{
		if (IS_USE_STRING_PTR && _strBuffer_const()==other._strBuffer_const()) 
			return *this;

		_setnull();
		if (other._strBuffer_const()==NULL)
		{			
			memcpy(mStrBuffer, other.mStrBuffer, STRING_BUFFER_LENGTH);			
		}		
		else
		{
			_setStrBuffer((char*)other._strBuffer_const());
			SetSpace(other.Space());
			AddUse();
		}

		return *this;
	}
	AString& operator = (const char* szString)
	{
		_set(szString);
		return *this;
	}
	AString& operator = (const wchar_t* szString);

	//AString& operator = (const int x);
	//AString& operator = (const Int64 x);

	//operator const char* () const
	//{
	//	return c_str();
	//}
	//operator bool () const{ return mStringBuffer!=NULL; }
	operator size_t () const
	{ 
		const char *key = c_str();
		int len = length();
		register uint nr=1, nr2=4; 
		while (len--) 
		{ 
			nr^= (((nr & 63)+nr2)*((uint) (uchar) *key++))+ (nr << 8); 
			nr2+=3; 
		} 
		return((uint) nr); 
	}

protected:

	typedef unsigned short UseType;
	typedef unsigned int SizeType;

	friend class DataStream;

	friend AString	Binary2String(const CHAR* pIn, UINT InLength, const char *preFix);

protected:
	union 
	{
		char		mStrBuffer[STRING_BUFFER_LENGTH+1];		// 最后字符如果不为 '\0' 为 'P'时，表示分配了指针空间 
		struct{
		const char	*mShowString;
		uint		mSize;
		};
	};

protected:
	UseType Use() const { if (IS_USE_STRING_PTR) return *(const UseType*)_strBuffer_const(); return 0; }

	// 分配的空间大小， 最好直接放在数组中
	SizeType Space() const { if (IS_USE_STRING_PTR) return *(SizeType*)(&mStrBuffer[8]); return STRING_BUFFER_LENGTH+1; }

	char* _strBuffer()
	{
		if (IS_USE_STRING_PTR)
			return ((char*)( *(size_t*)mStrBuffer ))-StringSpaceInfoSize;

		return NULL; 
	}

	const char* _strBuffer_const() const
	{
		if (IS_USE_STRING_PTR)
			return ((char*)( *(size_t*)mStrBuffer ))-StringSpaceInfoSize;

		return NULL; 
	}

	void _setStrBuffer(char *strBuff)
	{		
		*(size_t*)mStrBuffer = (size_t)(strBuff+StringSpaceInfoSize);
		mStrBuffer[STRING_BUFFER_LENGTH] = 'P';
	}

	void Alloc(SizeType size)
	{				
		_setnull();
		if (size>STRING_BUFFER_LENGTH)
		{
			size_t s = size + StringSpaceInfoSize;
			if (s<15)
				s = 15;

			AssertNote(s<INT_MAX, "字符空间不能大于[%d]", INT_MAX-StringSpaceInfoSize);
			char *ptr = (char*)ALLOCT_NEW(s);
			if (ptr==NULL)
			{
				AssertEx(0, "分配字符串空间失败");
				return;
			}
			_setStrBuffer(ptr);

#if DEBUT_STRING
			AString::gDebugLock.lock();
			gDebugString.insert(_strBuffer()+StringSpaceInfoSize);
			AString::gDebugLock.unlock();
#endif
			memset(ptr, 0, s);
			AddUse();
			SetSpace(s-StringSpaceInfoSize);
		}		
	}
	//void resize(size_t len){ Alloc(len); }

	void AddUse()
	{
		AssertNote(Use()<=0xFFFF, "引用数量不可超过 0xFFFF");
		*(UseType*)_strBuffer() = Use()+1;
	}

	void SetSpace(int s)
	{
		if (s<0) s = 0;
		*(SizeType*)(&mStrBuffer[8]) = s;
	}

	void _setnull();

public:
#if DEBUT_STRING
	static CsLock				gDebugLock;
	static std::set<char*>		gDebugString;
#endif


protected:
	void _set(const char* szString);

	char* _str()
	{
		if (IS_USE_STRING_PTR)
		{
			if (_strBuffer()!=NULL)
				return _strBuffer() + StringSpaceInfoSize;

			return NULL;
		}
		return mStrBuffer;
	}
	bool is() const { return _strBuffer_const()!=NULL; }

	size_t size() const { return Space(); }

public:
	static size_t _hash(const char *szString)
	{
		int len = strlen(szString);
		register uint nr=1, nr2=4; 
		while (len--) 
		{ 
			nr^= (((nr & 63)+nr2)*((uint) (uchar) *szString++))+ (nr << 8); 
			nr2+=3; 
		} 
		return((uint) nr); 
	}
};

struct AStringHash {
	size_t operator()(const AString& other) const { return AString::_hash(other.c_str()); }
};

#ifdef __LINUX__
extern int	MultiByteToWideChar(
	unsigned int  CodePage,
	unsigned long    dwFlags,
	const char*   lpMultiByteStr,
	int      cbMultiByte,
	wchar_t*  lpWideCharStr,
	int      cchWideChar);

extern int	WideCharToMultiByte(
	unsigned int  CodePage,
	unsigned long    dwFlags,
	const wchar_t*  lpWideCharStr,
	int      cchWideChar,
	char*   lpMultiByteStr,
	int      cbMultiByte,
	const char*   lpDefaultChar,
	int*  lpUsedDefaultChar);
#endif
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

#endif //_INCLUDE_AUTOSTRING_H_