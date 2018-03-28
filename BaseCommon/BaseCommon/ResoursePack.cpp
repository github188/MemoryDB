
#include "ResoursePack.h"
#include "AutoString.h"
#include "TableTool.h"
#include "Md5Tool.h"

#define USE_MAP_FAST_SERIALIZATION	0

//namespace Encrypt

	//-------------------------------------------------------------------------
	// 加解密数据
	const unsigned char encrypt_sbox[256] = 
	{
		0xe2,0x01,0xbc,0xb6,0xf7,0x93,0xd0,0x5c,0xa6,0xea,0x1d,0xb5,0x13,0x6c,0x62,0x6f,
		0xab,0x40,0x55,0x8d,0xae,0xd2,0xdb,0xb0,0xd4,0x33,0xa0,0xe9,0xcd,0x96,0xd1,0x8c,
		0x84,0xd3,0x34,0x92,0xb1,0xc5,0x61,0x6e,0x59,0x43,0x0a,0x81,0x54,0xa3,0x0f,0x4a,
		0xee,0xa2,0x0d,0xe3,0xf4,0xc6,0xaf,0x18,0x38,0xff,0xe6,0x6d,0x53,0x9e,0x78,0xa9,
		0x06,0xce,0x7b,0x56,0x29,0xad,0x75,0x11,0x74,0x17,0x58,0x36,0x2e,0x67,0x2f,0x19,
		0x86,0xf2,0x09,0x65,0x5e,0x79,0xfe,0x73,0x27,0x5d,0xf5,0x52,0xe4,0xe1,0xbd,0xdc,
		0x7c,0x90,0x2a,0xc2,0xde,0x5b,0xcf,0x3d,0x39,0xfa,0x15,0xa4,0x0c,0x89,0x44,0x35,
		0xc1,0x9f,0x12,0xf1,0x80,0x77,0x4d,0x5a,0x2b,0x91,0x64,0xc0,0xcb,0x04,0xd9,0xcc,
		0x98,0x5f,0x68,0x22,0x25,0x9b,0x50,0x83,0xdf,0x21,0x31,0x32,0xf9,0x85,0x99,0x0b,
		0xe7,0xb7,0x3a,0x02,0x1e,0xe0,0x28,0xa1,0xc9,0x47,0x46,0xaa,0x6b,0x69,0x70,0x3b,
		0x1b,0x41,0x49,0x0e,0x2d,0x23,0x20,0xa5,0x94,0xb3,0xf6,0xeb,0x3c,0xb9,0x8a,0x95,
		0x1a,0xb8,0xb4,0x7e,0xa7,0x05,0xbb,0x1c,0xbf,0x7a,0x4e,0xda,0xd5,0xc3,0xba,0x37,
		0xf8,0x72,0xc4,0xac,0xfc,0x1f,0x8f,0x97,0xdd,0x16,0x87,0x42,0x88,0xc7,0xe5,0xe8,
		0x03,0x4c,0x66,0x57,0x6a,0xd8,0x14,0x9a,0x26,0x3f,0x24,0x2c,0x9c,0x51,0x10,0xa8,
		0x07,0x82,0xb2,0x8e,0xd6,0xca,0xef,0xfb,0x08,0x76,0x4f,0x9d,0xd7,0x00,0x3e,0x60,
		0xbe,0xed,0x71,0xf0,0x8b,0xfd,0xc8,0xec,0x45,0x63,0x30,0x48,0xf3,0x7d,0x7f,0x4b,
	};


	const unsigned char encrypt_invsbox[256] = 
	{
		0xed,0x01,0x93,0xd0,0x7d,0xb5,0x40,0xe0,0xe8,0x52,0x2a,0x8f,0x6c,0x32,0xa3,0x2e,
		0xde,0x47,0x72,0x0c,0xd6,0x6a,0xc9,0x49,0x37,0x4f,0xb0,0xa0,0xb7,0x0a,0x94,0xc5,
		0xa6,0x89,0x83,0xa5,0xda,0x84,0xd8,0x58,0x96,0x44,0x62,0x78,0xdb,0xa4,0x4c,0x4e,
		0xfa,0x8a,0x8b,0x19,0x22,0x6f,0x4b,0xbf,0x38,0x68,0x92,0x9f,0xac,0x67,0xee,0xd9,
		0x11,0xa1,0xcb,0x29,0x6e,0xf8,0x9a,0x99,0xfb,0xa2,0x2f,0xff,0xd1,0x76,0xba,0xea,
		0x86,0xdd,0x5b,0x3c,0x2c,0x12,0x43,0xd3,0x4a,0x28,0x77,0x65,0x07,0x59,0x54,0x81,
		0xef,0x26,0x0e,0xf9,0x7a,0x53,0xd2,0x4d,0x82,0x9d,0xd4,0x9c,0x0d,0x3b,0x27,0x0f,
		0x9e,0xf2,0xc1,0x57,0x48,0x46,0xe9,0x75,0x3e,0x55,0xb9,0x42,0x60,0xfd,0xb3,0xfe,
		0x74,0x2b,0xe1,0x87,0x20,0x8d,0x50,0xca,0xcc,0x6d,0xae,0xf4,0x1f,0x13,0xe3,0xc6,
		0x61,0x79,0x23,0x05,0xa8,0xaf,0x1d,0xc7,0x80,0x8e,0xd7,0x85,0xdc,0xeb,0x3d,0x71,
		0x1a,0x97,0x31,0x2d,0x6b,0xa7,0x08,0xb4,0xdf,0x3f,0x9b,0x10,0xc3,0x45,0x14,0x36,
		0x17,0x24,0xe2,0xa9,0xb2,0x0b,0x03,0x91,0xb1,0xad,0xbe,0xb6,0x02,0x5e,0xf0,0xb8,
		0x7b,0x70,0x63,0xbd,0xc2,0x25,0x35,0xcd,0xf6,0x98,0xe5,0x7c,0x7f,0x1c,0x41,0x66,
		0x06,0x1e,0x15,0x21,0x18,0xbc,0xe4,0xec,0xd5,0x7e,0xbb,0x16,0x5f,0xc8,0x64,0x88,
		0x95,0x5d,0x00,0x33,0x5c,0xce,0x3a,0x90,0xcf,0x1b,0x09,0xab,0xf7,0xf1,0x30,0xe6,
		0xf3,0x73,0x51,0xfc,0x34,0x5a,0xaa,0x04,0xc0,0x8c,0x69,0xe7,0xc4,0xf5,0x56,0x39,
	};

	const unsigned char encrypt_xor_key[] = 
	{
		0x2f,0x84,0xd2,0xf1,0x12,0x98,0x07,0xd9,0xc2,0x5e,0xe1,0x22,0xad,0xae,0x52,0x75
	};

#define ENCRYPT_XOR_COM(x,xlen,BeginPlace)	if( (x)!=NULL ) \
	{ \
	\
	BYTE* t_pBuffer = (x); \
	UINT KeyLen = (UINT)sizeof(encrypt_xor_key); \
	for (UINT i = 0; i < (UINT)(xlen); i++) \
	{ \
	*t_pBuffer ^= encrypt_xor_key[(i+BeginPlace)%KeyLen]; \
	t_pBuffer++; \
		} \
		}

#define DECRYPT_XOR_COM(x,xlen,BeginPlace) ENCRYPT_XOR_COM(x,xlen,BeginPlace)

#define ENCRYPT_BOX_COM(x, xlen)  \
	{ \
	\
	BYTE* t_pBuffer = (BYTE*)(x); \
	for (UINT i = 0; i < (UINT)(xlen); i++) \
	{ \
	*t_pBuffer = encrypt_sbox[*t_pBuffer]; \
	t_pBuffer ++; \
		}\
		}

#define DECRYPT_BOX_COM(x, xlen)  \
	{ \
	\
	BYTE* t_pBuffer = (BYTE*)(x); \
	for (UINT i = 0; i < (UINT)(xlen); i++) \
	{ \
	*t_pBuffer = encrypt_invsbox[*t_pBuffer]; \
	t_pBuffer ++; \
		}\
		}

#define ONEGAME_ENCRYPT_COM(x, xlen)	{ ENCRYPT_XOR_COM(x, xlen, 0); ENCRYPT_BOX_COM(x, xlen); }

#define ONEGAME_DECRYPT_COM(x, xlen)	{ DECRYPT_BOX_COM(x, xlen); DECRYPT_XOR_COM(x, xlen, 0); }

	//-------------------------------------------------------------------------

	//-------------------------------------------------------------------------
bool ResourcesPack::load()
{
	if (!mPackDataStream.open(mPackFileName.c_str(), FILE_READ))
	{
		return false;
	}

	if ( mPackDataStream._read(&mPackHead, sizeof(PackHead)))              
	{
		if (!mPackDataStream.seek(mPackHead.mDataPos))
			return false;

		//资源文件信息索引
#if USE_MAP_FAST_SERIALIZATION
		mResourceList._resize(mPackHead.mCount);
		if (!mPackDataStream._read(mResourceList._data(), sizeof(EasyMap<int, ResourceInfo>::Value)*mPackHead.mCount))
			return false;
        size_t n = mPackHead.mCount;
		mResourceList._setCount(n, n);
#else
		for (int i=0; i<mPackHead.mCount; ++i )
		{
			int resourceIndex = 0;
			if (!mPackDataStream.read(resourceIndex))
				return false;

			ResourceInfo  info;
			if (!mPackDataStream._read(&info, sizeof(ResourceInfo)))
				return false;

			mResourceList.insert(resourceIndex, info);
		}
#endif
	}		

	return true;
}

AutoData ResourcesPack::open(const char *szPathFilename)
{
	AString strFileName = szPathFilename;
	strFileName.toLower();
	strFileName.replace('\\', '/');
	unsigned int fileId = strFileName.getKeyID();
	return open(fileId);
}

AutoData ResourcesPack::open(unsigned int fileId)
{
	ResourceInfo *pInfo = mResourceList.findPtr(fileId);

	if (pInfo==NULL)
		return AutoData();

	if (!mPackDataStream.seek(pInfo->mDataPos))
		return AutoData();

	AutoData resultData = MEM_NEW DataBuffer(pInfo->mSize);

	bool re = mPackDataStream._read(resultData->data(), pInfo->mSize);
	if (!re)
		return AutoData();

	resultData->setDataSize(pInfo->mSize);

	if (mPackHead.mbEncrypt)
		_DecryptData((void*)resultData->data(), resultData->dataSize());

	if (mPackHead.mbZip)
	{
		return _UnZipData(resultData);
	}

	return resultData;
}

unsigned char* ResourcesPack::loadResource(const char* szFileName, int* pSize)
{
	AString strFileName = szFileName;
	strFileName.toLower();
	strFileName.replace('\\', '/');
	unsigned int fileId = strFileName.getKeyID();

	return loadResource(fileId, pSize);
}

unsigned char* ResourcesPack::loadResource(int resourceIndex, int * pSize)
{
	ResourceInfo *pInfo = mResourceList.findPtr(resourceIndex);
	if (pInfo!=NULL)
	{
		if (!mPackDataStream.seek(pInfo->mDataPos))
			return false;

		unsigned char *szData = new unsigned char[pInfo->mSize];
		
		bool re = mPackDataStream._read(szData, pInfo->mSize);
		DSIZE dataS = pInfo->mSize;

		if (mPackHead.mbEncrypt)
			_DecryptData(szData, dataS);

		if (!mPackHead.mbZip)
		{
			*pSize = dataS;
			return szData;
		}

		// 解压
		DSIZE scrSize = *(DSIZE*)szData;

		unsigned char *scrData = NULL;
		if (scrSize==0)
		{
			scrData = new unsigned char[dataS-SIZE_LENGTH];
			memcpy(scrData, szData+SIZE_LENGTH, dataS-SIZE_LENGTH);
			*pSize = dataS-SIZE_LENGTH;
		}
		else
		{
			scrData = new unsigned char[scrSize];
			if (!DataBuffer::UnZip(szData+SIZE_LENGTH, dataS-SIZE_LENGTH, scrData, scrSize))
			{
				delete[] scrData;
				scrData = NULL;
				*pSize = 0;
			}
			else
				*pSize = scrSize;
		}
		delete[] szData;
		return scrData;

	}
	return NULL;
}

bool ResourcesPack::_EncryptData( void *scrData, DSIZE length )
{
	ONEGAME_ENCRYPT_COM((byte*)scrData, length);
	return true;
}


bool ResourcesPack::_DecryptData(void *scrData, DSIZE length)
{
	ONEGAME_DECRYPT_COM((byte*)scrData, length);	
	return true;
}

AutoData ResourcesPack::_UnZipData( AutoData scrData )
{
	// 此时的 scrData 如果需要解密, 已经完成解密
	AutoData resultData = MEM_NEW DataBuffer();
	DSIZE scrSize = 0;
	if (!scrData->read(scrSize))
		return AutoData();

	if (scrSize==0)
	{
		scrSize = scrData->dataSize()-SIZE_LENGTH;
		resultData->resize(scrSize);
		scrData->_read(resultData->data(), scrSize);
		resultData->setDataSize(scrSize);
	}
	else
	{
		HandDataBuffer scr = scrData;
		bool re = scr->UnZipData(resultData, 0, scrSize, SIZE_LENGTH, scrData->dataSize()-SIZE_LENGTH);
		if (!re)
		{
			TABLE_LOG("Error: Unzip data fail");
			return AutoData();
		}
	}
	return resultData;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
bool GenerateResourcesPack::GeneratePack(const char *szDestPathFileName, AString &errorInfo, DataStream *pResultPackData)
{
	DataStream  *pPackData = pResultPackData;

	FileDataStream  fileData;
	if (pPackData==NULL)
	{
		if (!fileData.open(szDestPathFileName, FILE_WRITE))
		{
			errorInfo = "File create fail";
			return false;
		}
		pPackData = &fileData;
	}
	else
		pPackData->clear(false);

	PackHead  head;
	strcpy(head.mVer, "ONE1.00");
	head.mbZip = mbZip;
	head.mbEncrypt = mEncrypt;
	head.mCount = mTempFileList.size();
	head.mDataPos = sizeof(PackHead);

	if (!pPackData->_write(&head, sizeof(head)))
	{
		errorInfo = "Write file head fail";
		return false;
	}

	int indexPos = sizeof(PackHead);
	indexPos += (sizeof(ResourceInfo)+sizeof(int))*mTempFileList.size();

	EasyMap<int, ResourceInfo>	tempResourceInfoList;

	for (int i=0; i<(int)mTempFileList.size(); ++i)
	{
		AutoData resData = mTempFileList.get(i);
		ResourceInfo info;
		info.mDataPos = indexPos;
		info.mSize = resData->dataSize();

		int resIndexID = mTempFileList.getKey(i).getKeyID();

		tempResourceInfoList.insert(resIndexID, info);
#if ! USE_MAP_FAST_SERIALIZATION
		if (!pPackData->write(resIndexID))
		{
			errorInfo = "Write resource id index fail";
			return false;
		}

		if (!pPackData->_write(&info, sizeof(info)))
		{
			errorInfo = "Write file info fail";
			return false;
		}
#endif
		indexPos += info.mSize;

		//errorInfo += mTempFileList.getKey(i).c_str();
		//errorInfo += "\r\n";

	}
#if USE_MAP_FAST_SERIALIZATION
	if (!pPackData->_write(tempResourceInfoList._data(), sizeof(EasyMap<int, ResourceInfo>::Value)*tempResourceInfoList.size()))
	{
		errorInfo = "Write resource info fail";
		return false;
	}
#endif
	AssertEx(pPackData->tell()==sizeof(PackHead) + (sizeof(ResourceInfo)+sizeof(int))*mTempFileList.size(), "检验写入错误");

	for (int i=0; i<(int)mTempFileList.size(); ++i)
	{
		AutoData resData = mTempFileList.get(i);
		if (resData->dataSize()<=0)
			continue;

		if (!pPackData->_write(resData->data(), resData->dataSize()))
		{
			errorInfo = "write resource data fail";
			return false;
		}
	}

	AssertEx(pPackData->tell()==indexPos, "验证包大小失败");

	// 更新MD5信息, 注意MD5计算不包括文件头
	if (pResultPackData!=NULL)
	{
		DSIZE endPos = pResultPackData->tell();	
		if (endPos>PACKHEAD_SIZE)
		{
			MD5	md(pResultPackData->data()+sizeof(PackHead), endPos-sizeof(PackHead));
			memcpy(head.mMD5, md.toString().c_str(), 32);
			head.mMD5[32] = '\0';
		}
		else
			memset(head.mMD5, 0, 33);

		pResultPackData->seek(0);
		pResultPackData->_write(&head, sizeof(PackHead));
		pResultPackData->seek(endPos);
	}

	return true;

}

AutoData GenerateResourcesPack::_EncryptOrZipData( AutoData scrData )
{
	AutoData resultData = scrData;
	if (mbZip)
	{
		resultData = MEM_NEW DataBuffer();
		HandDataBuffer scr = scrData;
		DSIZE s = scr->ZipData(resultData, SIZE_LENGTH, 0, 0, MAX_SPEED);
		if (s==0)
		{
			resultData->resize(scrData->dataSize()+SIZE_LENGTH);
			resultData->write((DSIZE)0);
			resultData->_write(scrData->data(), scrData->dataSize());
			resultData->setDataSize(scrData->dataSize()+SIZE_LENGTH);
		}
		else
		{
			resultData->seek(0);
			resultData->write((DSIZE)scrData->dataSize());
			resultData->setDataSize(SIZE_LENGTH+s);
		}
	}
	if (mEncrypt)
	{
		ONEGAME_ENCRYPT_COM((byte*)resultData->data(), resultData->dataSize());
	}
	return resultData;
}

//-------------------------------------------------------------------------

//-------------------------------------------------------------------------

bool EasyGenerateResourcesPack::GeneratePack( const char *szDestPathFileName, AString &errorInfo, DataStream *pResultPackData /*= NULL*/, int *pCount )
{
	DataStream *pDestData = pResultPackData;
	FileDataStream  fileData;
	if (pDestData==NULL)
	{
		if (!fileData.open(szDestPathFileName, FILE_WRITE))
		{
			errorInfo = "File create fail";
			return false;
		}
		pDestData = &fileData;
	}
	else
		pDestData->clear(false);
	pDestData->seek(0);
	pDestData->write((Int64)0);
	pDestData->write(mbZip);
	pDestData->write(mEncrypt);

	AutoTable indexTable = tBaseTable::NewBaseTable();
	indexTable->SetField("INDEX", FIELD_STRING, 0);
	indexTable->SetField("POS", FIELD_INT, 1);
	indexTable->SetField("SIZE", FIELD_INT, 2);
	//NOTE_LOG("-------------------------------------------------------");
	for (int i=0; i<(int)mTempFileList.size(); ++i)
	{
		AutoData resData = mTempFileList.get(i);
		ARecord indexRe = indexTable->CreateRecord(mTempFileList.getKey(i).c_str(), true);
		indexRe->set("POS", pDestData->tell());
		indexRe->set("SIZE", resData->dataSize());
		//MD5 md(resData->data(), resData->dataSize());
		//NOTE_LOG("%s %d, size =%d MD5 %s", mTempFileList.getKey(i).c_str(), pDestData->tell(), resData->dataSize(), md.toString().c_str());
		pDestData->_write(resData->data(), resData->dataSize());
	}		
	//NOTE_LOG("-------------------------------------------------------");
	int indexPos = pDestData->tell();
	if (!indexTable->SortSaveData(pDestData))
		return false;

	int size = pDestData->tell();
	pDestData->seek(0);
	pDestData->write(indexPos);
	pDestData->write(size);
	pDestData->seek(size);

	if (pCount!=NULL)
		*pCount = (int)mTempFileList.size();
	//indexTable->SaveCSV("D:/test_resource.csv");
	return true;
}


AutoData EasyGenerateResourcesPack::_EncryptOrZipData(AutoData scrData)
{

	if (mEncrypt)
	{
		ONEGAME_ENCRYPT_COM((byte*)scrData->data(), scrData->dataSize());
	}
	return scrData;
}

//-------------------------------------------------------------------------*/
AutoData EasyResourcesPack::loadResource(const char *fileIndex)
{
	if (!mResourcesIndex || !mPackData)
		return AutoData();

	AString path = fileIndex;
	path.toLower();
	path.replace('\\', '/');

	AutoRecord indexRe = mResourcesIndex->GetRecord(path.c_str());
	if (indexRe)
	{
		int pos = indexRe->get("POS");
		int size = indexRe->get("SIZE");
		AutoData d = MEM_NEW DataBuffer(size);
		if (mPackData->seek(pos) && mPackData->_read(d->data(), size))
		{
			d->setDataSize(size);
			if (mbEnrypt)
			ONEGAME_DECRYPT_COM((byte*)d->data(), size);
			return d;
		}
	}

	return AutoData();
}

int EasyResourcesPack::Export( const char *path, AString &errorInfo )
{
	int nAllCount = 0;
	AString destPath = path;
	for (TableIt tIt(mResourcesIndex); tIt._Have(); tIt._Next())
	{
		ARecord r = tIt.getCurrRecord();
		AString fileIndex = r->getIndexData().string();

		AutoData d = loadResource(fileIndex.c_str());
		if (d)
		{
			FileDataStream f((destPath+fileIndex).c_str(), FILE_CREATE);
			if (f._write(d->data(), d->dataSize()))
				++nAllCount;
		}
		else
		{
			errorInfo += " Fail load resource >";
			errorInfo += fileIndex;
			errorInfo += ", ";
		}
	}
	return nAllCount;
}
