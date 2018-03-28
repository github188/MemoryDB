
#ifndef _INCLUDE_FILEDATASTREAM_H_
#define _INCLUDE_FILEDATASTREAM_H_

#include "stdio.h"

#include "DataStream.h"


#if __WINDOWS__
	#include <tchar.h>
#endif

#include "BaseCommon.h"

#include "DataBuffer.h"


using namespace std;

//-------------------------------------------------------------------------
// 通用文件操作
// 都以二进制方式进行操作
//-------------------------------------------------------------------------
enum FILE_OPERATE
{
	FILE_READ,
	FILE_WRITE,
	FILE_CREATE,	
	FILE_READ_WRITE,
};

//文件工具,可以帮助一个文件指针自动关闭,
class BaseCommon_Export FileDataStream : public DataStream
{

public:
	~FileDataStream();
	FileDataStream(){ mFile = NULL; }
	
	FileDataStream( const char* fileName, FILE_OPERATE openmode );

public:
	operator bool (){ return NULL!=mFile; }

	bool open(const char* fileName, FILE_OPERATE openmode);

	UInt64 tell64();
	bool seek64(UInt64 offset);
	UInt64 size64();

public:
	virtual bool empty(void){ return NULL==mFile || size()==0; }
	inline virtual void clear(bool){ close(); }

	void close(void);

	virtual DSIZE tell(void) const;

	virtual bool seek(DSIZE absolutePosition);
	virtual bool setDataSize(DSIZE dataSize){ AssertEx(0, "不可用"); return false; }

	virtual DSIZE size(void) const;
	inline  DSIZE lastSize(void){ return 100*1024*1024; }
	virtual DSIZE dataSize(void) const { return size(); }
	virtual char* data(void);

	virtual bool resize(DSIZE destSize, char fullValue = 0);
	virtual void initData(byte initVal) { }

	bool _write( void *scrData, DSIZE dataSize );
	bool _read( void *destData, DSIZE readSize );


	inline virtual bool writeWString(const wchar_t *wString){ return false; }
	inline virtual const wchar_t* readWString(DSIZE &resultLength){ return false; }

	virtual bool readLine(AString &resultString) override;

public:
	FileDataStream & operator = ( const FileDataStream& fObj )
	{
		this->~FileDataStream();
		mFile = fObj.mFile;
		((FileDataStream*)&fObj)->_setNull();
		return *this;
	}

	AutoData readAllData();

	void flush();

private:
	size_t  mFile;
	void _setNull(void){mFile = NULL;}

public:
	static AString GetBaseFileName(const char *sFullName);
	static AString GetFileName(const char *sFullName);
	static AString GetFileExtName(const char *sFullName);
	static AString GetPathFileNameWithoutExt(const char *sFullName);
	static AString GetPathName(const char *sFullName);
};


#endif //_INCLUDE_FILEDATASTREAM_H_