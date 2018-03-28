
#ifndef _INCLUDE_RESOURSEPACK_H_
#define _INCLUDE_RESOURSEPACK_H_

#include "BaseCommon.h"
#include "FileDataStream.h"
#include "EasyMap.h"
#include "BaseTable.h"

#define PACKHEAD_SIZE	(sizeof(PackHead))

const int SIZE_LENGTH  = sizeof(DSIZE);
//-------------------------------------------------------------------------
struct PackHead
{
	char	mVer[32];
	char	mMD5[33];
	int		mCount;
	bool	mbZip;
	bool	mbEncrypt;
	int		mDataPos;

public:
	PackHead()
	{
		Reset();
	}

	void Reset()
	{
		mCount = 0;
		mbZip = false;
		mbEncrypt = false;
		mDataPos = 0;	
		mVer[0] = '\0';
		memset(mMD5, 0, 33);
	}
};

struct ResourceInfo
{
	int		mDataPos;
	int		mSize;
};
typedef EasyHash<int, ResourceInfo>	ResourceInfoList;

//-------------------------------------------------------------------------
class BaseCommon_Export ResourcesPack : public Base<ResourcesPack>
{
public:
	ResourcesPack(){}
	ResourcesPack(const char *szPackFileName )
		: mPackFileName(szPackFileName)
	{
	}
	~ResourcesPack()
	{
		unload();
	}

	const char* getVer(){ return mPackHead.mVer; }
	const char* getMD5(){ return mPackHead.mMD5; }

public:
	bool load();
	bool load( const char *packFileName )
	{
		mPackFileName = packFileName;	
		return load();
	}


	void unload()
	{				
		mPackFileName = "";
		mResourceList.clear(false);
		mPackHead.Reset();
	}

	AutoData open(const char *szPathFilename);
	AutoData open(unsigned int fileId);

	unsigned char* loadResource(const char* pszFileName, int * pSize);
	unsigned char* loadResource(int resourceIndex, int * pSize);

public:
	static bool _EncryptData(void *scrData, DSIZE length);
	static bool _DecryptData(void *scrData, DSIZE length);

protected:
	AutoData _UnZipData(AutoData scrData);

protected:
	EasyString			mPackFileName;
	FileDataStream		mPackDataStream;
	PackHead			mPackHead;
	ResourceInfoList	mResourceList;
};


//-------------------------------------------------------------------------
// 用于制作生成资源包
//-------------------------------------------------------------------------

class BaseCommon_Export GenerateResourcesPack
{
public:
	GenerateResourcesPack()
		: mbZip(false)
		, mEncrypt(false)
	{

	}

	GenerateResourcesPack(bool bZip, bool bEncrypt)
		: mbZip(bZip)
		, mEncrypt(bEncrypt)
	{

	}

	virtual ~GenerateResourcesPack(){}

	void InitReset(bool bZip, bool bEncrypt)
	{
		mbZip = bZip;
		mEncrypt = bEncrypt;
		mTempFileList.clear(false);
	}

public:
	void AppendResourceData(const char* pathFileName, AutoData resourceData)
	{
		AString path = pathFileName;
		path.toLower();
		path.replace('\\', '/');
		AutoData d = _EncryptOrZipData(resourceData);
		mTempFileList.insert(path, d);
	}

	virtual bool GeneratePack(const char *szDestPathFileName, AString &errorInfo, DataStream *pResultPackData = NULL);


protected:	
	virtual AutoData _EncryptOrZipData(AutoData scrData);


public:
	EasyMap<AString, AutoData>		mTempFileList;
	bool mbZip;
	bool mEncrypt;
};

//-------------------------------------------------------------------------
// 使用表格方式索引的简单的资源包
//-------------------------------------------------------------------------
class BaseCommon_Export EasyResourcesPack : public AutoBase
{
	friend class EasyGenerateResourcesPack;

public:
	bool load(const char *packFileName)
	{
		FileDataStream *f = MEM_NEW FileDataStream(packFileName, FILE_READ);
		AutoData tempData = f;
		if (*f)		
			return load(f);
		mPackData.setNull();
		return false;
	}

	bool load (DataStream *packData)
	{
		packData->seek(0);			

		int indexPos = 0, size = 0;
		READ(packData, indexPos);
		READ(packData, size);

		if (!packData->seek(indexPos))
			return false;

		mResourcesIndex = tBaseTable::NewBaseTable();
		if (!mResourcesIndex->LoadFromData(packData))
			return false;

		packData->seek(sizeof(int)*2);
		packData->read(mbZip);
		packData->read(mbEnrypt);

		mPackData = packData;
		return true;
	}

	AutoData loadResource(const char *fileIndex);

	AutoTable GetResourcesIndex(){ return  mResourcesIndex; }

	int Export(const char *path, AString &errorInfo);

	bool IsZip() const { return mbZip; }
	bool IsEnrypt() const { return mbEnrypt; }

public:
	EasyResourcesPack()
		: mbZip(false)
		, mbEnrypt(false)
	{

	}

protected:
	AutoTable		mResourcesIndex;

	AutoData		mPackData;
	bool			mbZip;
	bool			mbEnrypt;
};
typedef Auto<EasyResourcesPack> ADataPack;
//-------------------------------------------------------------------------
// 用于制作生成简易的资源包
//-------------------------------------------------------------------------

class BaseCommon_Export EasyGenerateResourcesPack : public GenerateResourcesPack
{

public:
	virtual bool GeneratePack(const char *szDestPathFileName, AString &errorInfo, DataStream *pResultPackData = NULL, int *pCount = NULL);


protected:	
	virtual AutoData _EncryptOrZipData(AutoData scrData);

};

#endif //_INCLUDE_RESOURSEPACK_H_