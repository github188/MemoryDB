
#include "AutoString.h"
#include "TableTool.h"

#ifndef uint32
	typedef unsigned int uint32;
#endif


#if DEBUT_STRING
	std::set<char*>	AString::gDebugString;
	CsLock			AString::gDebugLock;
#endif

#ifdef __LINUX__
// if destStr is NULL, then return result need length.
/*int WideCharToMultiByte(int option, int arg, const wchar_t* w, int wLen, char* destStr, int destLen, void *p, void *p2 )
{
    return 0;
}

int MultiByteToWideChar(int option, int arg, const char* scrString, int len, wchar_t *destString, int destLen)
{
    return 0;
}
*/
#define CP_UTF8 65001
//#include <iconv.h>
#include <stdio.h>
int	MultiByteToWideChar(
                        unsigned int  CodePage,
                        unsigned long    dwFlags,
                        const char*   lpMultiByteStr,
                        int      cbMultiByte,
                        wchar_t*  lpWideCharStr,
                        int      cchWideChar)
{
    /*
    printf("Warn: linux system no finish [MultiByteToWideChar] function ");
    return 0;
    
	char* pIn = (char*)lpMultiByteStr;
	char* pOut = (char*)lpWideCharStr;
	if (!pOut)
	{
		return wcslen((wchar_t*)pOut);
	}
	iconv_t cd = iconv_open("INTERNAL","UTF-8");;
    
	if ((iconv_t)-1 == cd)
	{
		AssertEx(0, "iconv_open() error");
		return -1;
	}
	if ((size_t)-1 == (iconv(cd, &pIn, (size_t *)&cbMultiByte, &pOut,(size_t *)&cchWideChar)))
	{
		AssertEx(0, "iconv() error");
	}
	iconv_close(cd);*/
	return -1;
}
int	WideCharToMultiByte(
                        unsigned int  CodePage,
                        unsigned long    dwFlags,
                        const wchar_t*  lpWideCharStr,
                        int      cchWideChar,
                        char*   lpMultiByteStr,
                        int      cbMultiByte,
                        const char*   lpDefaultChar,
                        int*  lpUsedDefaultChar)
{
    /*
    printf("Warn: linux system no finish [WideCharToMultiByte] function ");
    return 0;
    
	char* pIn = (char*)lpWideCharStr;
	char* pOut = (char*)lpMultiByteStr;
	if (!pOut)
	{
		return strlen(pOut);
	}
	iconv_t cd = iconv_open("UTF-8","UCS-2-INTERNAL");;
    
	if ((iconv_t)-1 == cd)
	{
		AssertEx(0, "iconv_open() error");
		return -1;
	}
	if ((size_t)-1 == (iconv(cd, &pIn, (size_t *)&cchWideChar, &pOut,(size_t *)&cbMultiByte)))
	{
		AssertEx(0, "iconv() error");
	}
	iconv_close(cd);*/
	return -1;
}
#else
#include <windows.h>
#endif

#include "DataBuffer.h"


AString& AString::operator=( const wchar_t* wString )
{
	_setnull();
	int nLen = WideCharToMultiByte(0, 0, wString, -1, NULL, 0, NULL, NULL);
	if (nLen>0)
	{
		Alloc(nLen);
		WideCharToMultiByte(0, 0, wString, -1, _str(), nLen, NULL, NULL); 					
	}
	return *this;
}

//AString& AString::operator=( const int x )
//{
//	*this = STRING(x);
//	return *this;
//}
//
//AString& AString::operator=( const Int64 x )
//{
//	*this = STRING(x);
//	return *this;
//}



AString& AString::operator+=( const wchar_t *wString )
{
	if (wString)
	{
		int len1 = WideCharToMultiByte(0, 0, wString, -1, NULL, 0, NULL, NULL);

		if (len1>0)
		{
			AString temp = *this;
			
			Alloc(temp.length()+len1);
			memcpy(_str(), temp.c_str(), temp.length());
			WideCharToMultiByte(0, 0, wString, -1, _str()+temp.length(), len1, NULL, NULL); 	
		}
	}
	return *this;
}

AString& AString::operator+=( int nVal )
{	
	char temp[16];
#ifdef __LINUX__
    sprintf(temp, "%d", nVal);
#else
	_itoa_s(nVal, temp, 16, 10);
#endif
	return link(temp);
}

AString& AString::operator+=( float fVal )
{
	char temp[64];
#ifdef __LINUX__
    sprintf(temp, "%f", fVal);
#else
	//if (fVal==0)
	//	strcpy(temp, "0.0");
	//else
		sprintf_s(temp, 64, "%f", fVal);
#endif
	return link(temp);
}

AString& AString::operator+=( double dVal )
{
	char temp[64];
	sprintf(temp, "%.16f", dVal);	
	return link(temp);
}

AString& AString::operator += (UInt64 uValU64)
{
	char temp[48];
	sprintf(temp, "%llu", uValU64);
	return link(temp);
}

bool AString::getU64(UInt64  &uResult)
{
	return sscanf( c_str(), "%llu", &uResult)==1;
}


AString AString::operator+( const char *szString )
{
	AString temp = c_str();
	return temp.link(szString);
}

AString AString::operator+( const AString &str )
{
	AString temp = *this;
	return temp.link(str.c_str());
}

AString AString::operator+( const wchar_t *wString )
{
	AString temp = *this;
	temp += wString;
	return temp;
}

AString AString::operator+( int nVal )
{
	AString temp = *this;
	StringTool d;
	
	return temp.link(d.ToString(nVal));
}

AString AString::operator+( float fVal )
{
	AString temp = *this;
	StringTool d;
	
	return temp.link(d.ToString(fVal));
}
WString AString::getWString(uint pagecode) const
{
	WString temp;
	int nLen = MultiByteToWideChar(pagecode, 0, c_str(), -1, NULL, 0);		
	if (nLen>0)
	{
		temp = new wchar_t[nLen];
		memset(temp.getPtr(), 0, nLen*sizeof(wchar_t));
		MultiByteToWideChar(pagecode, 0, c_str(), -1, temp.getPtr(), nLen); 			
	}
	return temp;
}


typedef unsigned int uint32;

uint32 UniCharToUTF8(wchar_t UniChar, char *OutUTFString);
unsigned int UTF8StrToUnicode( const char* UTF8String, uint32 UTF8StringLength, wchar_t* OutUnicodeString, uint32 UnicodeStringBufferSize );


AString AString::getUTF8( const char *scr, int pageCode)
{
	AString temp = scr;
	WString w = temp.getWString(pageCode);

	wchar_t *wStr = w.getPtr();

	int len = wcslen(wStr);
	int c = 0;
	Buffer d(len*4+1);	// NOTE: UniCharToUTF8 may be return 4
	for (int i=0; i<len; ++i)
	{
		c += UniCharToUTF8(wStr[i], d.data()+c);
	}
	d.data()[c] = 0;
	AString re = d.data();
	return re;
	
	//int nLen = WideCharToMultiByte(CP_UTF8, 0, w.getPtr(), -1, NULL, 0, NULL, NULL);
	//if (nLen>0)
	//{
	//	AutoMemString re;
	//	re.resize(nLen);

	//	WideCharToMultiByte(CP_UTF8, 0, w.getPtr(), -1, re._str(), nLen, NULL, NULL); 					
	//	return re;
	//}	
	return AString();
}

AString AString::getANIS( const char *szUTF8 )
{
	if (szUTF8==NULL)
		return AString();
//#if __WINDOWS__
//	WString temp;
//	int nLen = MultiByteToWideChar(CP_UTF8, 0, szUTF8, -1, NULL, 0);		
//	if (nLen>0)
//	{
//		temp = new wchar_t[nLen];
//		memset(temp.getPtr(), 0, nLen*sizeof(wchar_t));
//		MultiByteToWideChar(CP_UTF8, 0, szUTF8, -1, temp.getPtr(), nLen);
//
//		AutoMemString re = temp.getPtr();
//		return re;
//	}
//#else
	wchar_t  *temp  = new wchar_t[strlen(szUTF8)+1];
	int len = UTF8StrToUnicode(szUTF8, strlen(szUTF8), temp, strlen(szUTF8)+1);
	temp[len] = 0;
	
	AString re = temp;
	delete[] temp;
	return re;
//#endif
	return AString();
}

void AString::Format( const char* pFormat, ... )
{
#ifdef __LINUX__
    va_list argptr;

    va_start(argptr, pFormat);
    _setnull();
    int len = vsnprintf(NULL, 0, pFormat, argptr);
	if (len>0)
	{
        Alloc(len+1);
        vsprintf(_str(), pFormat, argptr);
    }
	va_end(argptr);
#else
    va_list vaList;
	va_start(vaList, pFormat);
	_setnull();
	int len = _vscprintf(pFormat, vaList);
	if (len>0)
	{
		Alloc(len+1);
		_vsnprintf_s ( _str(), len+1, len, pFormat, vaList);	
		//char *temp = new char[len+1];
		//_vsnprintf_s ( temp, len+1, len, pFormat, vaList);	
		//temp[len] = '\0';
		//_set(temp);
	}
	va_end(vaList);
#endif
}

void AString::Format( va_list &valist, const char *pFormat )
{
#ifdef __LINUX__

	_setnull();
	int len = vsnprintf(NULL, 0, pFormat, valist);
	if (len>0)
	{
		Alloc(len+1);
		vsprintf(_str(), pFormat, valist);
	}
	va_end(valist);
#else
	_setnull();
	int len = _vscprintf(pFormat, valist);
	if (len>0)
	{
		Alloc(len+1);
		_vsnprintf_s ( _str(), len+1, len, pFormat, valist);	

	}
	va_end(valist);
#endif
}


void AString::linkInt64( Int64 i64 )
{
#ifdef __LINUX__
    AssertEx(0, "linkInt64 Error");
#else
	char temp[24];
	_i64toa(i64, temp, 10);
	link(temp);
#endif
}

int AString::Split( const char *scrString, Array<AString> &resultStringList, const char *delims, int maxSplits )
{
	AString str = scrString;
	int c = 0;
	while (true)
	{
		bool bHave = strstr(str.c_str(), delims)!=NULL;
        if (maxSplits>0 && resultStringList.size()>=maxSplits-1)
        {
            if (bHave || str!="")
                resultStringList.push_back(str);
            break;
        }
		AString left = str.SplitLeft(delims);
		if (left.empty() && !bHave)
		{
			if (str!="")
				resultStringList.push_back(str);
			break;
		}
		else
			resultStringList.push_back(left);
	}
	return resultStringList.size();
	//std::string str = scrString;
	//unsigned int numSplits = 0;

	//// Use STL methods 
	//size_t start, pos;
	//start = 0;
	//do 
	//{
	//	++numSplits;

	//	pos = str.find_first_of(delims, start);
	//	if (pos == start)
	//	{
	//		// Do nothing
	//		start = pos + 1;
	//	}
	//	else if (pos == std::string::npos || (maxSplits && numSplits == maxSplits))
	//	{
	//		// Copy the rest of the string
	//		resultStringList.push_back( AString(str.substr(start).c_str()) );
	//		break;
	//	}
	//	else
	//	{
	//		// Copy up to delimiter
	//		resultStringList.push_back( AString(str.substr(start, pos - start).c_str()) );
	//		start = pos + 1;
	//	}
	//	// parse up to next real data
	//	start = str.find_first_not_of(delims, start);
	//	
	//} while (pos != std::string::npos);

	//return numSplits;
}

void AString::toLower()
{
	for (int i=0; i<(int)length(); ++i)
	{
		*(_str()+i) = tolower(*(c_str()+i));
	}
}


const char* AString::toUTF8()
{
	*this = getUTF8(c_str());
	return c_str();
}


const char* AString::toANIS()
{
	*this = getANIS(c_str());
	return c_str();
}

int AString::getKeyID() const
{
	return MAKE_INDEX_ID(c_str());
}

int AString::replace( char scrChar, char destChar )
{
	int result = 0;
	for (int i=0; i<(int)length(); ++i)
	{
		if ( *(c_str()+i)==scrChar)
		{
			++result;
			*(_str()+i) = destChar;
		}
	}
	return result;
}


void AString::replace(const char *szScrStr, const char *szDest)
{
	if (strstr(c_str(), szScrStr)==NULL)
		return;
	int scrLen = strlen(szScrStr);
	AString temp;
	if (IS_USE_STRING_PTR && Use()>1)
		temp.set(c_str());
	else
		temp = *this;
	free();
	const char *sz = temp.c_str();
	while (true)
	{
		const char *p = strstr(sz, szScrStr);
		if (p)
		{
			if (p-sz>0)
			{
				*(char*)p = '\0';
				(*this) += sz;
			}
			(*this) += szDest;
			sz += (p-sz)+scrLen;			
		}
		else
		{
			(*this) += sz;
			break;
		}
	}
}


void AString::replaceFirst(const char *szScrStr, const char *szDest)
{
	const char *p = strstr(c_str(), szScrStr);
	if (p==NULL)
		return;
	int scrLen = strlen(szScrStr);
	AString temp;

	const char *sz = c_str();
	if (p-sz>0)
	{
		temp.set(sz, (DSIZE)(p-sz));
	}
	temp += szDest;
	sz = p+scrLen;
	temp += sz;
	*this = temp;
}


void AString::replaceLast(const char *szScrStr, const char *szDest)
{
	const char *p = strstr(c_str(), szScrStr);
	if (p==NULL)
		return;
	int scrLen = strlen(szScrStr);
	AString temp;

	const char *sz = c_str();
	while (true)
	{
		const char *pN = strstr(sz, szScrStr);
		if (pN)
		{
			p = pN;
			sz = p+scrLen;
		}
		else
			break;
	}
	if (p-c_str()>0)
	{
		temp.set(c_str(), (DSIZE)(p-c_str()));
	}
	temp += szDest;
	sz = p+scrLen;
	temp += sz;
	*this = temp;
}

AString AString::SplitLeft( const char *splitString )
{
	const char *beginString = strstr(c_str(), splitString);

	if (beginString!=NULL)
	{
		DSIZE len = (DSIZE)(beginString-c_str());
		
		AString r;
		r.set(c_str(), len);
		
		if (IS_USE_STRING_PTR && Use()>1)
		{
			AString temp = beginString+strlen(splitString);
			*this = temp;
		}
		else
		{
			DSIZE x = len + strlen(splitString);
			memmove(_str(), beginString+strlen(splitString), length()-x+1);
		}
		return r;
	}

	return AString();
}

AString AString::SplitBlock(const char s1/*= '{'*/, const char s2 /*= '}'*/, bool bSkipSplitChar)
{
	int num = 0;	
	char szTemp[2] = {s1, '\0'};
	const char* sz = strstr(c_str(), szTemp);
	if (sz==NULL)
		return AString();

	sz += 1;

	const char *beginStr = sz;

	while (true)
	{
		if (*sz=='\0')
			break;
		else if (*sz==s2)
		{
			if (num<=0)
			{
				DSIZE len = (DSIZE)(sz-beginStr);

				AString result;
				if (bSkipSplitChar)
					result.set(beginStr-1, len+2);
				else
					result.set(beginStr, len);

                if (!IS_USE_STRING_PTR || Use()<=1)
                {
				    DSIZE lastSize = Space()-len-2;
				    memmove(_str(), beginStr + len +1, length()-len+1);
                }
                else
				{					 
                    AString temp = beginStr + len +1;
					*this = temp;
				}
				return result;
			}
			--num;
		}
		else if (*sz==s1)
			++num;

		sz += 1;
	}
	return AString();
}

AString& AString::link( const char *szString )
{
	if (szString!=NULL)
	{
		size_t len1 = strlen(szString);

		if (len1<=0)
		{
			return *this;
		}

		DSIZE nowLen = length();
		DSIZE lastLen = nowLen+len1+1;

		if (IS_USE_STRING_PTR)
		{				
			if (Use()>1 || (DSIZE)Space()<lastLen)
			{
				AString temp = *this;
				Alloc(lastLen);
				memcpy(_str(), temp.c_str(), temp.length()+1);
			}			
		}
		else
		{
			if (lastLen>STRING_BUFFER_LENGTH)
			{
				AString temp = *this;
				Alloc(lastLen);
				memcpy(_str(), temp.c_str(), temp.length()+1);
			}
		}
#ifdef __WINDOWS__
		strcpy_s(_str()+nowLen, len1+1, szString);
#else
		strncpy(_str()+nowLen, szString, len1+1);
#endif		
	}
	return *this;
}

void AString::set( const char *sz, DSIZE len )
{
	DSIZE s = strlen(sz);
	if (s<len)
		len = s;

	if (IS_USE_STRING_PTR)
	{
		if ( (Use()!=1 || (DSIZE)Space()<len+1))
			Alloc(len+1);
	}
	else if (len+1>STRING_BUFFER_LENGTH)
		Alloc(len+1);

	strncpy_s(_str(), len+1, sz, len);
}

//void AString::resize( size_t len )
//{
//	AssertEx( len<0xffff , "不支持长度超过65535的字符串!");
//
//	if (mASTool!=NULL && mASTool->mCount<=1)
//	{		
//		if (len>mASTool->mSize)
//		{				
//			mASTool->resize(len);			
//		}
//		else
//		{
//			memset(mASTool->mString, 0, mASTool->mSize);
//		}
//		return;
//	}
//
//	_setnull();
//	mASTool = MEM_NEW ASTool(len);
//
//#if DEBUT_STRING
//	gDebugString.insert(mASTool);
//#endif
//}

void AString::_set( const char* szString )
{
	if (szString==NULL)
	{
		setNull();
		return;
	}

	size_t len = strlen(szString);
	len += 1;
	if (IS_USE_STRING_PTR)
	{
		if (Use()==1)
		{
			if (Space()<len)
				Alloc(len);
		}
		else
			Alloc(len);
	}
	else
	{
		if (len>STRING_BUFFER_LENGTH)
			Alloc(len);
	}
#ifdef __WINDOWS__
	strcpy_s(_str(), len, szString);
#else
	strncpy(_str(), szString, len);
#endif

}

void AString::_setnull()
{
	if (IS_USE_STRING_PTR)
	{
		if (Use()-1==0)
		{
#if DEBUT_STRING
			CsLockTool	l(AString::gDebugLock);
			if (gDebugString.erase(_strBuffer()+StringSpaceInfoSize)<=0)
			{				
				AssertEx(0, "重复释放");
			}
#endif
			Allot::freePtr(_strBuffer());
		}
		else
			*(UseType*)_strBuffer() = Use()-1;
	}
	memset(mStrBuffer, 0, sizeof(AString));

}


//-------------------------------------------------------------------------

//typedef unsigned __int32 uint32;

uint32 UniCharToUTF8(wchar_t UniChar, char *OutUTFString)
{

	uint32 UTF8CharLength = 0;

	if (UniChar < 0x80)
	{ 
		if ( OutUTFString )
			OutUTFString[UTF8CharLength++] = (char)UniChar;
		else
			UTF8CharLength++;
	}
	else if(UniChar < 0x800)
	{
		if ( OutUTFString )
		{
			OutUTFString[UTF8CharLength++] = 0xc0 | ( UniChar >> 6 );
			OutUTFString[UTF8CharLength++] = 0x80 | ( UniChar & 0x3f );
		}
		else
		{
			UTF8CharLength += 2;
		}
	}
	else if(UniChar < 0x10000 )
	{
		if ( OutUTFString )
		{
			OutUTFString[UTF8CharLength++] = 0xe0 | ( UniChar >> 12 );
			OutUTFString[UTF8CharLength++] = 0x80 | ( (UniChar >> 6) & 0x3f );
			OutUTFString[UTF8CharLength++] = 0x80 | ( UniChar & 0x3f );
		}
		else
		{
			UTF8CharLength += 3;
		}
	}
	else if( UniChar < 0x200000 )
	{
		if ( OutUTFString )
		{
			OutUTFString[UTF8CharLength++] = 0xf0 | ( (int)UniChar >> 18 );
			OutUTFString[UTF8CharLength++] = 0x80 | ( (UniChar >> 12) & 0x3f );
			OutUTFString[UTF8CharLength++] = 0x80 | ( (UniChar >> 6) & 0x3f );
			OutUTFString[UTF8CharLength++] = 0x80 | ( UniChar & 0x3f );
		}
		else
		{
			UTF8CharLength += 4;
		}

	}
	AssertEx(UTF8CharLength<=4, "UniCharToUTF8 must <= 4");
	return UTF8CharLength;
}



//当OutUnicodeString为NULL时, 可以进行输出的Unicode字符串长度探测
//
//

unsigned int UTF8StrToUnicode( const char* UTF8String, uint32 UTF8StringLength, wchar_t* OutUnicodeString, uint32 UnicodeStringBufferSize )
{
	uint32 UTF8Index = 0;
	uint32 UniIndex = 0;

	while ( UTF8Index < UTF8StringLength )
	{
		unsigned char UTF8Char = UTF8String[UTF8Index];

		if ( UnicodeStringBufferSize != 0 && UniIndex >= UnicodeStringBufferSize )
			break;

		if ((UTF8Char & 0x80) == 0)
		{
			const uint32 cUTF8CharRequire = 1;

			// UTF8字码不足
			if ( UTF8Index + cUTF8CharRequire > UTF8StringLength )
				break;

			if ( OutUnicodeString )
			{
				wchar_t& WideChar = OutUnicodeString[UniIndex];

				WideChar = UTF8Char;
			}

			UTF8Index++;

		}
		else if((UTF8Char & 0xE0) == 0xC0)  ///< 110x-xxxx 10xx-xxxx
		{
			const uint32 cUTF8CharRequire = 2;

			// UTF8字码不足
			if ( UTF8Index + cUTF8CharRequire > UTF8StringLength )
				break;

			if ( OutUnicodeString )
			{
				wchar_t& WideChar = OutUnicodeString[UniIndex];
				WideChar  = (UTF8String[UTF8Index + 0] & 0x3F) << 6;
				WideChar |= (UTF8String[UTF8Index + 1] & 0x3F);
			}

			UTF8Index += cUTF8CharRequire;
		}
		else if((UTF8Char & 0xF0) == 0xE0)  ///< 1110-xxxx 10xx-xxxx 10xx-xxxx
		{
			const uint32 cUTF8CharRequire = 3;

			// UTF8字码不足
			if ( UTF8Index + cUTF8CharRequire > UTF8StringLength )
				break;

			if ( OutUnicodeString )
			{
				wchar_t& WideChar = OutUnicodeString[UniIndex];

				WideChar  = (UTF8String[UTF8Index + 0] & 0x1F) << 12;
				WideChar |= (UTF8String[UTF8Index + 1] & 0x3F) << 6;
				WideChar |= (UTF8String[UTF8Index + 2] & 0x3F);
			}

			UTF8Index += cUTF8CharRequire;
		}
		else if((UTF8Char & 0xF8) == 0xF0)  ///< 1111-0xxx 10xx-xxxx 10xx-xxxx 10xx-xxxx
		{
			const uint32 cUTF8CharRequire = 4;

			// UTF8字码不足
			if ( UTF8Index + cUTF8CharRequire > UTF8StringLength )
				break;

			if ( OutUnicodeString )
			{
				wchar_t& WideChar = OutUnicodeString[UniIndex];

				WideChar  = (UTF8String[UTF8Index + 0] & 0x0F) << 18;
				WideChar  = (UTF8String[UTF8Index + 1] & 0x3F) << 12;
				WideChar |= (UTF8String[UTF8Index + 2] & 0x3F) << 6;
				WideChar |= (UTF8String[UTF8Index + 3] & 0x3F);
			}

			UTF8Index += cUTF8CharRequire;
		}
		else ///< 1111-10xx 10xx-xxxx 10xx-xxxx 10xx-xxxx 10xx-xxxx
		{
			const uint32 cUTF8CharRequire = 5;

			// UTF8字码不足
			if ( UTF8Index + cUTF8CharRequire > UTF8StringLength )
				break;

			if ( OutUnicodeString )
			{
				wchar_t& WideChar = OutUnicodeString[UniIndex];

				WideChar  = (UTF8String[UTF8Index + 0] & 0x07) << 24;
				WideChar  = (UTF8String[UTF8Index + 1] & 0x3F) << 18;
				WideChar  = (UTF8String[UTF8Index + 2] & 0x3F) << 12;
				WideChar |= (UTF8String[UTF8Index + 3] & 0x3F) << 6;
				WideChar |= (UTF8String[UTF8Index + 4] & 0x3F);
			}

			UTF8Index += cUTF8CharRequire;
		}


		UniIndex++;
	}

	return UniIndex;
}

//-------------------------------------------------------------------------
CHAR		Value2Ascii(CHAR in)
{
	__ENTER_FUNCTION
		switch(in) 
	{
		case 0:
			return '0';
			break;
		case 1:
			return '1';
		case 2:
			return '2';
			break;
		case 3:
			return '3';
			break;
		case 4:
			return '4';
			break;
		case 5:
			return '5';
			break;
		case 6:
			return '6';
			break;
		case 7:
			return '7';
			break;
		case 8:
			return '8';
			break;
		case 9:
			return '9';
			break;
		case 10:
			return 'A';
			break;
		case 11:
			return 'B';
			break;
		case 12:
			return 'C';
			break;
		case 13:
			return 'D';
			break;
		case 14:
			return 'E';
			break;
		case 15:
			return 'F';
			break;
		default:
			Assert(FALSE);
			return '?';
			break;
	}

	__LEAVE_FUNCTION

		return '?';
}

CHAR Ascii2Value(CHAR in)
{
	__ENTER_FUNCTION
		switch(in) 
	{
		case '0':
			return 0;
			break;
		case '1':
			return 1;
		case '2':
			return 2;
			break;
		case '3':
			return 3;
			break;
		case '4':
			return 4;
			break;
		case '5':
			return 5;
			break;
		case '6':
			return 6;
			break;
		case '7':
			return 7;
			break;
		case '8':
			return 8;
			break;
		case '9':
			return 9;
			break;
		case 'A':
			return 10;
			break;
		case 'B':
			return 11;
			break;
		case 'C':
			return 12;
			break;
		case 'D':
			return 13;
			break;
		case 'E':
			return 14;
			break;
		case 'F':
			return 15;
			break;
		default:
			Assert(FALSE);
			return '?';
			break;
	}

	__LEAVE_FUNCTION

		return '?';
}


// 这个函数非常不安全，没有安全检测，希望小心使用 [杨文鸽 2011/1/18]
//	添加以下函数的注释 [6/23/2010 王玉波]
//	参数说明：	pIn			输入参数，一段内存数据 
//				InLength	输入参数，这段内存数据的长度（字节）
//				pOut		输出参数，转化后的字符串存储在这个位置
//	功能说明：	把输入的内存数据每个字节的4Bit转化为字符'0'到'F',即每个字节转化为两个字符（4Bit一个）
//				序列化到输出缓冲区中,变成了一个字符串
//				
//BOOL	Binary2String(const CHAR* pIn, UINT InLength, CHAR* pOut)
//{
//	__ENTER_FUNCTION
//
//		if(InLength==0)
//		{
//			return FALSE;
//		}
//		UINT iOut = 0;
//
//
//		for(UINT i = 0 ;i<InLength;i++)
//		{
//
//			pOut[iOut] = Value2Ascii(((unsigned char)pIn[i]&0xF0)>>4);
//			iOut++;
//			pOut[iOut] = Value2Ascii(pIn[i]&0x0F);
//			iOut++;
//
//
//		}
//		return TRUE;
//
//		__LEAVE_FUNCTION
//
//			return FALSE;
//}
// 把二进制转换成字符串，且可以自动增长容量，返回处理后的数据长度 [杨文鸽 2011/1/18]
AString	Binary2String(const CHAR* pIn, UINT InLength, const char *preFix)
{
	__ENTER_FUNCTION

		if(InLength==0)
		{
			return AString();
		}

		int out = 0;
		if (preFix!=NULL)
			out = strlen(preFix);

		AString  result;

		size_t newLen = InLength*2+1;	

		result.Alloc(newLen + out);

		if (preFix!=NULL)
        {
#ifdef __LINUX__
            strcpy(result._str(), preFix);
#else
			strcpy_s(result._str(), newLen+out, preFix);
#endif
        }

		for(UINT i = 0 ;i<InLength;i++)
		{			
			result._str()[out++] = Value2Ascii(((unsigned char)pIn[i]&0xF0)>>4);			
			result._str()[out++] = Value2Ascii(pIn[i]&0x0F);			
		}		
		AssertEx(out<(int)result.size(), "必须小于字符串分配长度");
		result._str()[out++] = '\0';

		AssertEx(out==result.size(), "Error: Binary2String memory space error.");

		return result;

		__LEAVE_FUNCTION

			return AString();
}


BOOL String2Binary(const CHAR* pIn,UINT InLength,CHAR* pOut,UINT OutLimit,UINT& OutLength)
{
	__ENTER_FUNCTION

		if(InLength==0)
		{
			return FALSE;
		}

		UINT iOut = 0;
		UINT i;
		for( i = 0 ;i<InLength-1;)
		{
			if((pIn[i]=='\\') && (pIn[i+1]=='0'))
			{
				pOut[iOut]	=	'\0';
				iOut++;
				i+=2;
				continue;
			}
			else if((pIn[i]=='\'') && (pIn[i+1]=='\''))
			{
				pOut[iOut]	=	'\'';
				iOut++;
				i+=2;
				continue;
			}
			else if((pIn[i]=='\\') && (pIn[i+1]=='\"'))
			{
				pOut[iOut]	=	'\"';
				iOut++;
				i+=2;
				continue;
			}
			else if((pIn[i]=='\\') && (pIn[i+1]=='\\'))
			{
				pOut[iOut]	=	'\\';
				iOut++;
				i+=2;
				continue;
			}
			else if(pIn[i]=='\0')
			{
				break;
			}

			pOut[iOut] = pIn[i];
			iOut++;
			i++;

			if(iOut>=OutLimit)
				break;
		}
		OutLength = iOut;
		return TRUE;

		__LEAVE_FUNCTION
			return FALSE;
}

