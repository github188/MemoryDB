/********************************************************************
created:	2011/05/24
created:	24:5:2011   23:14
filename: 	h:\2011_02_27\Code\common\DataBase\EasyString.h
file path:	h:\2011_02_27\Code\common\DataBase
file base:	EasyString
file ext:	h
author:		杨文鸽

purpose:	
*********************************************************************/

#ifndef _INCLUDE_EASYSTRING_H_
#define _INCLUDE_EASYSTRING_H_

#include "AutoString.h"

typedef AString EasyString;
typedef Array<EasyString, 8> StringArray;

//#include "BaseCommon.h"
//
//#include "Array.h"
//
//#define STRING_NICE				1
//
//#if STRING_NICE
//
//#define STRING_DEFALUT_LENGTH	15		// 最小不可于指针长度+1 (64位 >=9, 32位>=5 )
////--------------------------------------------------------------------------------------
//// 内存池字符串
////--------------------------------------------------------------------------------------
//struct EasyString 
//{
//
//	char	mString[STRING_DEFALUT_LENGTH];
//
//public:
//	EasyString()
//	{
//		memset(mString, 0, STRING_DEFALUT_LENGTH);		
//	}
//
//	~EasyString()
//	{
//		_free();
//	}
//
//public:
//	EasyString(const char *szString)
//	{
//		memset(mString, 0, STRING_DEFALUT_LENGTH);
//		*this = szString;
//	}
//	EasyString(const EasyString &ps)
//	{
//		memset(mString, 0, STRING_DEFALUT_LENGTH);		
//		set(ps.c_str());
//	}
//
//	//operator bool () const { return !empty(); }
//
//	EasyString& operator = (const char* szString) { set(szString); return *this; }
//
//	EasyString& operator = (const EasyString& szString) { set(szString.c_str()); return *this; }
//
//	bool operator == ( const EasyString& other) const
//	{
//		return strcmp(c_str(), other.c_str())==0;
//	}
//	bool operator != ( const EasyString& other) const
//	{
//		return strcmp(c_str(), other.c_str())!=0; 
//	}
//
//	bool operator == ( const char* other) const
//	{
//		return strcmp(c_str(), other)==0;
//	}
//	bool operator != ( const char *other) const
//	{
//		return strcmp(c_str(), other)!=0;
//	}
//
//	bool operator < (const EasyString& other) const
//	{
//		return strcmp(c_str(), other.c_str())<0;
//	}
//
//	bool operator < (const char *other) const
//	{
//		return strcmp(c_str(), other)<0;
//	}
//
//	bool operator > (const EasyString& other) const
//	{
//		return strcmp(c_str(), other.c_str())>0;
//	}
//
//	bool operator > (const char *other) const
//	{
//		return strcmp(c_str(), other)>0;
//	}
//
//public:
//	operator const char* () const
//	{
//		return c_str();
//	}
//
//	bool empty() const { return length()==0; }
//
//	const char* c_str() const 
//	{ 
//		if (mString[STRING_DEFALUT_LENGTH-1]!='\0') 		
//			return (const char*)(*(size_t*)mString); 
//
//		return mString; 
//	}
//
//	//char* _str() { if (mChar) return mChar; else return mString; }
//	size_t length() const { return strlen(c_str()); }
//
//	void free(){ _free(); }
//
//	void _free()
//	{
//		if (mString[STRING_DEFALUT_LENGTH-1]!='\0') 
//		{
//			StringTool::__deleteStringEx((void*)_ptr(), length());
//		}
//
//		memset(mString, 0, STRING_DEFALUT_LENGTH);
//
//	}
//
//	void set( const char* sz, size_t len = 0 )
//	{
//		if (_ptr()==sz)
//			return;
//
//		_free();
//
//		if (sz==NULL)
//		{
//			return;
//		}
//
//		if (len==0)
//			len = strlen(sz);
//
//		if (len==0)
//		{
//			return;
//		}
//
//		if (len<STRING_DEFALUT_LENGTH-1)
//		{
//			strncpy(&(mString[0]), sz, len);
//			mString[len] = '\0';
//			return;
//		}
//
//		_newChar(len+1);
//		strncpy(_ptr(), sz, len);
//		_ptr()[len] = '\0';
//		return;		
//	}
//
//	void swap(EasyString &other)
//	{
//		//char *p = other.mChar;
//		//other.mChar = mChar;
//		//mChar = p;
//
//		char szTemp[STRING_DEFALUT_LENGTH];
//		memcpy(szTemp, mString, STRING_DEFALUT_LENGTH);
//		memcpy(mString, other.mString, STRING_DEFALUT_LENGTH);
//		memcpy(other.mString, szTemp, STRING_DEFALUT_LENGTH);
//	}
//
//	void set(int fVal)
//	{
//		//_newChar(12);
//		_free();
//		_itoa_s(fVal, mString, STRING_DEFALUT_LENGTH, 10);		
//	}
//
//	void set(float fVal)
//	{
//		_free();
//		//_newChar(16);
//		sprintf_s(mString, STRING_DEFALUT_LENGTH, "%.7f", fVal);		
//	}
//
//	void set(UInt64 uVal8)
//	{
//		_free();
//		//_newChar(24);
//		sprintf_s(mString, STRING_DEFALUT_LENGTH, "%llu", uVal8);
//	}
//
//protected:
//	void _newChar(DSIZE len)
//	{
//		_free();
//		size_t s = len;
//		char *mChar = (char*)StringTool::__newStringEx(s);
//		_setPtr(mChar);
//		
//	}
//
//	char *_ptr()
//	{
//		if (mString[STRING_DEFALUT_LENGTH-1] != '\0')
//			return (char*)( *(size_t*)mString );
//
//		return mString;
//	}
//
//	void _setPtr(char *strPtr)
//	{
//		size_t v = (size_t)strPtr;
//		*(size_t*)mString = v;
//
//		mString[STRING_DEFALUT_LENGTH-1] = 'P';
//	}
//
//};
//
//#else
////-------------------------------------------------------------------------
//// 使用小内存池
////-------------------------------------------------------------------------
//struct EasyString 
//{
//	char*	mChar;
//
//public:
//	EasyString()
//	{
//		mChar = NULL;
//	}
//
//	~EasyString()
//	{
//		_free();
//	}
//
//public:
//	EasyString(const char *szString)
//	{
//		mChar = NULL;
//		*this = szString;
//	}
//	EasyString(const EasyString &ps)
//	{
//		mChar = NULL;
//		set(ps.c_str());
//	}
//
//	operator bool () const { return !empty(); }
//
//	EasyString& operator = (const char* szString) { set(szString); return *this; }
//
//	EasyString& operator = (const EasyString& szString) { set(szString.c_str()); return *this; }
//
//	bool operator == ( const EasyString& other) const
//	{
//		return strcmp(c_str(), other.c_str())==0;
//	}
//	bool operator != ( const EasyString& other) const
//	{
//		return strcmp(c_str(), other.c_str())!=0; 
//	}
//
//	bool operator == ( const char* other) const
//	{
//		return strcmp(c_str(), other)==0;
//	}
//	bool operator != ( const char *other) const
//	{
//		return strcmp(c_str(), other)!=0;
//	}
//
//	bool operator < (const EasyString& other)
//	{
//		return strcmp(c_str(), other.c_str())<0;
//	}
//
//	bool operator > (const EasyString& other)
//	{
//		return strcmp(c_str(), other.c_str())>0;
//	}
//
//public:
//	operator const char* () const
//	{
//		return c_str();
//	}
//
//	bool empty() const { return (NULL==mChar); }
//
//	const char* c_str() const { if (NULL!=mChar) return mChar; return ""; }
//	size_t length() const { return strlen(c_str()); }
//
//	void free(){ _free(); }
//
//	void _free()
//	{
//		if (NULL==mChar)
//			return;
//		size_t len = length();
//		//if (len==0)
//		//{
//		//	mChar = NULL;
//		//	return;
//		//}
//		//StringTool::__deleteString(StringTool::__getPosIndex(len+1), (void**)&mChar);
//		StringTool::__deleteStringEx((void*)mChar, len+1);
//		mChar = NULL;
//	}
//
//	void set( const char* sz )
//	{
//		if (sz==NULL)
//		{
//			_free();
//			return;
//		}
//		size_t len = strlen(sz);
//		if (len==0)
//		{
//			_free();
//			return;
//		}
//
//		_newChar(len+1);
//		strncpy(mChar, sz, len+1);
//		return;		
//	}
//
//	void swap(EasyString &other)
//	{
//		char *p = other.mChar;
//		other.mChar = mChar;
//		mChar = p;
//	}
//
//	void set(int fVal)
//	{
//		_newChar(12);
//
//		_itoa_s(fVal, mChar, 12, 10);		
//	}
//
//	void set(float fVal)
//	{
//		_newChar(16);
//		sprintf_s(mChar, 16, "%.7f", fVal);		
//	}
//
//	void set(UInt64 uVal8)
//	{
//		_newChar(24);
//		sprintf_s(mChar, 24, "%llu", uVal8);
//	}
//
//protected:
//	void _newChar(DSIZE len)
//	{
//		if (mChar!=NULL)
//		{
//			DSIZE nowLen = strlen(mChar);
//			if (len<nowLen)
//				return;
//
//			//if (len-nowLen<BASE_BLOCK_SIZE)
//			{
//				nowLen = Allot::getSize(mChar);
//				if (len<nowLen)
//					return;
//			}
//		}
//
//		_free();
//		mChar = (char*)StringTool::__newStringEx(len);
//	}
//};
////-------------------------------------------------------------------------
//#endif
//
//typedef Array<EasyString> StringArray;


#endif //_INCLUDE_EASYSTRING_H_