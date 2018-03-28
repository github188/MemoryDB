//#include "DataStream.h"


inline bool DataStream::writeData( DataStream *scrData, DSIZE writeLength )
{
	if (writeLength==0)
		return _write(&writeLength, sizeof(DSIZE));

	if (scrData==NULL)	
	{
		writeLength = 0;
		return _write(&writeLength, sizeof(DSIZE));
	}

	if (scrData->size()<writeLength)
		return false;

	bool re = _write(&writeLength, sizeof(DSIZE));

	if (writeLength>0 && re)
		return _write(scrData->data(), writeLength);

	return re;
}

const int _MAX_BUFFER_READ_SIZE	= 512*1024*1024;

inline bool DataStream::readData( DataStream *destData )
{
	AssertEx(destData!=NULL, "获取数据目标 destData 为空");

	DSIZE len = 0;
	if (read(len))
	{
		if (len>0)
		{
			if (len<=_MAX_BUFFER_READ_SIZE)
			{
				if (destData->size()<len)
					if (!destData->resize(len))
					{
						ERROR_LOG("Resize data fail >%d", len);
						return false;
					}

				if ( _read(destData->data(), len) )
				{
					destData->seek(len);
					destData->setDataSize(len);
					return true;
				}			

				destData->clear();
				return false;
			}
			else
			{
				ERROR_LOG("DataStream::readData len more then [%d]", _MAX_BUFFER_READ_SIZE);
				return false;
			}
		}
		return true;
	}
	return false;
}

inline bool DataStream::writeString( const AString &scr )
{
	return writeString(scr.c_str());
}

//inline bool DataStream::writeString( const EasyString &easyString )
//{
//	return writeString(easyString.c_str());
//}

typedef unsigned short	StrLenType;

inline bool DataStream::writeString( const char* scrString)
{
	DSIZE scrLength = (DSIZE)strlen(scrString);
	AssertNote(scrLength<=0xffff, "序列化字符串最大支持长度65535");
	if (scrLength>0xFFFF)
		return false;
	bool b = write((StrLenType)scrLength);
	AssertEx(b, "保存字符串长度失败");
	if (scrLength>0)
		return _write((void*)scrString, scrLength);

	return true;
}

inline bool DataStream::readString( AString &resultString )
{
	StrLenType reLen = 0;
	if (!read(reLen))
		return false;

	if (reLen>0)
	{
		resultString.Alloc(reLen+1);
		if ( !_read(resultString._str(), reLen) )
		{
			resultString.setNull();
			return false;
		}
	}
	else
		resultString.setNull();

	return true;
}

inline AString DataStream::readString( void )
{
	AString resultString;

	StrLenType reLen = 0;
	if (!read(reLen))
		return resultString;

	if (reLen>0)
	{
		resultString.Alloc(reLen+1);
		bool b = _read(resultString._str(), reLen);
		AssertEx(b, "读取字符串失败");
	}

	return resultString;
}

//inline bool DataStream::readString( EasyString &resultString )
//{
//	StrLenType reLen = 0;
//	if (!read(reLen))
//		return false;
//
//	resultString.free();
//	if (reLen>0)
//	{
//		if (lastSize()>=reLen)
//		{
//			resultString.set((const char*)nowData(), reLen);			
//			bool re = move((int)reLen);
//			AssertEx(re, "位置标应该正确移动");
//			return true;
//		}		
//		else
//			return false;
//	}	
//
//	return true;
//}

//inline const char* DataStream::readString( DSIZE &resultLength )
//{
//	const char* szResult = (const char*)nowData();
//	if (szResult==NULL)
//		return NULL;
//
//	resultLength = strlen( szResult )+1;
//	// last must have '\0', so < last size.
//	if (resultLength<=lastSize())
//	{
//		move((int)resultLength);
//		return szResult;
//	}
//
//	return NULL;
//}

inline bool DataStream::_write( void *scrData, DSIZE dataSize )
{
	if (lastSize()<dataSize)
		if (!resize(tell()+dataSize))
			return false;

	AssertEx(lastSize()>=dataSize, "空间应该分配够用");
	memcpy(nowData(), scrData, dataSize);
	bool re = move((int)dataSize);
	AssertEx(re, "位置标应该正确移动");
	return re;
}

inline bool DataStream::_read( void *destData, DSIZE readSize )
{
	if (lastDataSize()>=readSize)
	{
		memcpy(destData, nowData(), readSize);
		bool re = move((int)readSize);
		AssertEx(re, "位置标应该正确移动");
		return true;
	}
	return false;
}


inline void* DataStream::nowData( void )
{
	if (tell()<size()) 
		return (char*)data() + tell(); 
	else 
		return NULL;
}

inline bool DataStream::move( int offSize )
{
	if (offSize==0)
		return true;

	if (offSize<0)
	{
		if (tell() >= (DSIZE)(-offSize))
		{
			seek(tell()+offSize);
			return true;
		}
	}
	else 
	{
		if (tell()+offSize <= size() )
		{
			seek(tell()+offSize);
			return true;
		}
	}
	return false;
}


inline bool DataStream::writeWString(const wchar_t *wString)
{	
	DSIZE length = (DSIZE)wcslen(wString) + 1;
	return _write((void*)wString, length * sizeof(wchar_t));
}

inline const wchar_t* DataStream::readWString(DSIZE &resultLength)
{
	const wchar_t* szResult = (const wchar_t*)nowData();
	resultLength = (DSIZE)((wcslen( szResult )+1) * sizeof(wchar_t));
	// last must have '\0', so < last size.
	if (resultLength<=lastDataSize())
	{
		move((int)resultLength);
		return szResult;
	}

	return NULL;
}