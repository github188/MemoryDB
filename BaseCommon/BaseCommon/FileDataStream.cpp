
#include "FileDataStream.h"
#include <stdlib.h>

#define USE_FILE	1

#include "Assertx.h"
#include "TableTool.h"

#	include <fcntl.h>
#	include <sys/stat.h>

#if USE_FILE

FileDataStream::FileDataStream( const char* fileName, FILE_OPERATE openmode )
	: mFile(NULL)
{
	open(fileName, openmode);
}

bool FileDataStream::open( const char* fileName, FILE_OPERATE openmode )
{
	close();

	const char *szMode = "rb";
	switch (openmode)
	{
	case FILE_CREATE:
	case FILE_WRITE:
		szMode = "wb";
		break;

	case FILE_READ_WRITE:
		szMode = "rb+";
		break;

	case FILE_READ:
	default:		
		szMode = "rb";
		break;
	}

	mFile = (size_t)fopen(fileName, szMode);
	if (mFile!=NULL)
		seek(0);
	return mFile!=NULL;

}


UInt64 FileDataStream::tell64()
{
	return (UInt64)_ftelli64((FILE*)mFile);
}


bool FileDataStream::seek64(UInt64 offset)
{
	return _fseeki64((FILE*)mFile, offset, SEEK_SET)==0;
}


UInt64 FileDataStream::size64()
{
	if (NULL!=mFile)
	{
		UInt64 current = tell64();
		if (_fseeki64((FILE*)mFile, 0, SEEK_END) == 0)
		{
			UInt64 re = _ftelli64((FILE*)mFile);
			if (_fseeki64((FILE*)mFile, current, SEEK_SET)!=0)				
				ERROR_LOG("Restore now pos fail [llu]", current);

			return re;
		}
		ERROR_LOG("File get size fail.");
	}
	return 0;
}

FileDataStream::~FileDataStream()
{
	close();
}


void FileDataStream::close( void )
{
	if ( mFile!=NULL ) { fclose((FILE*)mFile); mFile = NULL;}
}

DSIZE FileDataStream::tell( void ) const
{
	if (NULL!=mFile)
		return ftell((FILE*)mFile);

	return -1;
}

bool FileDataStream::seek( DSIZE absolutePosition )
{
	if (NULL!=mFile)
	{
		fseek((FILE*)mFile, absolutePosition, SEEK_SET);
		if ( tell()==absolutePosition )		
			return true;
		else
		{
			TABLE_LOG("File seek fail.");
		}
	}
	return false;
}

DSIZE FileDataStream::size( void ) const
{
	if (NULL!=mFile)
	{
		DSIZE current = tell();
		if (fseek((FILE*)mFile, 0, SEEK_END) == 0)
		{
			DSIZE re = ftell((FILE*)mFile);
			if (fseek((FILE*)mFile, current, SEEK_SET)==0)				
				return re;
		}
		
	
		TABLE_LOG("File get size fail.");
		
	}
	return 0;
}

bool FileDataStream::_write( void *scrData, DSIZE dataSize )
{
	//if (lastSize()<dataSize)
	//if (!resize(tell()+dataSize))
	//		return false;

	//AssertEx(lastSize()>=dataSize, "空间应该分配够用");		

	//bool re = move(dataSize);
	//AssertEx(re, "位置标应该正确移动");
	//return re;
	if (mFile==NULL)
		return false;
	return fwrite(scrData, 1, dataSize, (FILE*)mFile)==dataSize;
}

bool FileDataStream::_read( void *destData, DSIZE readSize )
{
	//if (lastSize()>=readSize)
	//{
	//	memcpy(destData, nowData(), readSize);
	//	bool re = move(readSize);
	//	AssertEx(re, "位置标应该正确移动");
	//	return true;
	//}
	//return false;
	if (mFile==NULL)
		return false;

	return fread(destData, 1, readSize, (FILE*)mFile)==readSize;
}

bool FileDataStream::resize( DSIZE destSize, char fullValue /*= 0*/ )
{
	AssertEx(0, "File stream not can resize.");
	return false;
}

char* FileDataStream::data( void )
{
	AssertEx(0, "FileDataStream not can use char* data()!");
	return NULL;
}


bool FileDataStream::readLine(AString &resultString)
{
	if (mFile==0)
		return false;

	char  data_buf[4096];
	if (fgets( data_buf, 4096,  (FILE*)mFile)!=NULL)
	{
		char* p = strstr(data_buf, "\r\n");
		if (p==NULL)
			p = strstr(data_buf, "\n");
		if (p!=NULL)
		{
			*p = '\0';		
			resultString = data_buf;
		}
        else
            resultString = data_buf;
        return true;
	}
	return false;
}

AutoData FileDataStream::readAllData()
{
	DSIZE s = size();
	AutoData d = MEM_NEW DataBuffer(s);
	if (_read(d->data(), s))
	{
		d->setDataSize(s);
		return d;
	}

	return AutoData();
}


#else
#include "FileAPI.h"


using namespace FileAPI;

FileDataStream::FileDataStream( const char* fileName, FILE_OPERATE openmode )
{
	open(fileName, openmode);
}

 bool FileDataStream::open( const char* fileName, FILE_OPERATE openmode )
{
#ifdef __WINDOWS__
	INT flag = _O_BINARY;
	INT mode = 0;
	switch (openmode)
	{
	case FILE_CREATE:
		flag |= _O_WRONLY | _O_CREAT;
		mode = _S_IREAD | _S_IWRITE;
		break;

	case FILE_WRITE:
		flag |= _O_WRONLY;
		break;

	case FILE_READ_WRITE:
		flag |= _O_RDWR;
		break;

	case FILE_READ:
	default:		
		flag |= _O_RDONLY;
		break;

	}

	mFile = open_ex(fileName, flag, mode);
	switch (mFile)
	{
	case EACCES:
		AssertEx(0, "文件被独占使用");
		break;

	case EEXIST:
		AssertEx(0, "文件已经存在");
		break;

	case EINVAL:
		AssertEx(0, "打开参数错误");
		break;

	case EMFILE:
		AssertEx(0, "打开文件数量过多");
		break;

	case ENOENT:
		AssertEx(0, "文件不存在,或路径错误");
		break;

	default:
		if (mFile>0)
			return true;
	}
#else
	INT flag = 0;
	switch (openmode)
	{
	case FILE_CREATE:
		flag |= O_WRONLY | O_CREAT ;
	
		break;

	case FILE_WRITE:
		flag |= O_WRONLY;
		break;

	case FILE_READ_WRITE:
		flag |= O_RDWR;
		break;

	case FILE_READ:
	default:		
		flag |= O_RDONLY;
		break;

	}
	mFile = open_ex(fileName, flag);

	if (mFile>NULL)
		return true;

#endif
	mFile = NULL;

	AString info = "file open fail > ";
	info += fileName;
	TABLE_LOG(LOG_ERROR, info.c_str() );
	//#pragma warning(push)
	//#pragma warning(disable:4996)
	//		mFile = _tfopen( fileName.c_str(),openmode.c_str() ); 
	//#pragma warning(pop)

	return false;

}

FileDataStream::~FileDataStream()
{
	if (mFile!=NULL) close_ex(mFile);
}


void FileDataStream::close( void )
{
	if ( mFile!=NULL ) { close_ex(mFile);mFile = NULL;}
}

DSIZE FileDataStream::tell( void ) const
{
	if (NULL!=mFile)
		return tell_ex(mFile);

	return -1;
}

bool FileDataStream::seek( DSIZE absolutePosition )
{
	if (NULL!=mFile)
	{
		if (lseek_ex(mFile, absolutePosition, SEEK_SET)!=-1L)
			return tell()==absolutePosition;
		else
		{
			TABLE_LOG(LOG_ERROR, "File seek fail.");
		}
	}
	return false;
}

DSIZE FileDataStream::size( void ) const
{
	if (NULL!=mFile)
	{
		DSIZE current = tell();
		if (lseek_ex(mFile, 0, SEEK_END) != -1L)
		{
			DSIZE re = tell_ex(mFile);
			if (lseek_ex(mFile, current, SEEK_SET) != -1L)				
				return re;
		}
		else
		{
			TABLE_LOG(LOG_ERROR, "File seek fail.");
		}
	}
	return 0;
}

bool FileDataStream::_write( void *scrData, DSIZE dataSize )
{
	//if (lastSize()<dataSize)
	//if (!resize(tell()+dataSize))
	//		return false;

	//AssertEx(lastSize()>=dataSize, "空间应该分配够用");		

	//bool re = move(dataSize);
	//AssertEx(re, "位置标应该正确移动");
	//return re;
	return write_ex(mFile, scrData, dataSize)==dataSize;
}

bool FileDataStream::_read( void *destData, DSIZE readSize )
{
	//if (lastSize()>=readSize)
	//{
	//	memcpy(destData, nowData(), readSize);
	//	bool re = move(readSize);
	//	AssertEx(re, "位置标应该正确移动");
	//	return true;
	//}
	//return false;
	AssertEx(mFile!=NULL, "文件尚未成功打开");
	return read_ex(mFile, destData, readSize)==readSize;
}

bool FileDataStream::resize( DSIZE destSize, char fullValue /*= 0*/ )
{
	AssertEx(0, "File stream not can resize.");
	return false;
}

char* FileDataStream::data( void )
{
	AssertEx(0, "FileDataStream not can use char* data()!");
	return NULL;
}

AutoData FileDataStream::readAllData()
{
	DSIZE s = size();
	AutoData d = MEM_NEW DataBuffer(s);
	if (_read(d->data(), s))
		return d;

	return AutoData();
}



#endif


void FileDataStream::flush()
{
	if (mFile!=NULL) 
		fflush((FILE*)mFile);
}


AString FileDataStream::GetBaseFileName(const char *sFullName)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath(sFullName, drive, dir, fname, ext);
	AString packName = fname;
	return packName;
}

AString FileDataStream::GetFileName(const char *sFullName)
{
	//char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath(sFullName, drive, dir, fname, ext);
	AString packName = fname;
	packName += ext;
	return packName;
}

AString FileDataStream::GetFileExtName(const char *sFullName)
{
	//char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath(sFullName, drive, dir, fname, ext);

	return ext;
}

AString FileDataStream::GetPathFileNameWithoutExt(const char *sFullName)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath(sFullName, drive, dir, fname, ext);
	AString pathFile = drive;
	pathFile += dir;
	pathFile += fname;

	return pathFile;
}

AString FileDataStream::GetPathName(const char *sFullName)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath(sFullName, drive, dir, fname, ext);
	AString pathFile = drive;
	pathFile += dir;
	return pathFile;
}
