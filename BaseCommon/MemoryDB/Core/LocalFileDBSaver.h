/*/-------------------------------------------------------------------------
	DB��������Ż����� (�ɼ�����Դ�洢����, ��֧�����ݼ�ʱ�޸�ͬ��, �����ϲ���Ҫ������д������, ȫ���ύ��ϵͳͬ��, ��ȫϵͳ�ϸ�)
	1 ���´������, ʹ���ڴ��ļ�ӳ���, Ƶ��д���޸�С���ڴ�ʱ, �ٶȷǳ���(100000 ��=15����)
	2 ʵ��FileDBRecord�̳�MemoryDBRecord �����ļ��ڴ�ָ��, ʵ��FileDBTable�̳�MemoryDBTable, ʹ��FileDBRecord, ������, �����ļ�ָ��
	3 �ڼ�¼��Ҫ�޸�ʱ, ���޸ĵ��ֶν����ļ�ָ�붨λ (��¼�ļ�ָ��+4�ֽ��ֶ�У��+ǰ���ֶ�getDBDataLength()), FileDBTable�б�Ҫʹ��ר���ֶ�, ���������ֶ�ƫ��ָ��λ��, STRING = 32, TABLE = 64K
	4 �ֶ�����ֱ��memcpy��ӳ��λ��
	5 ���ݱ��ṹ����Ϊ�ı���ʽ(CSV), �����趨���ֲ�
	6 ���ʹ�ú�̨�Ļ�, �ɴ����涨����������, �����з����ļ��м�¼��Ҫ�ĳ��Ȼ���, ���������ü�¼���ļ�ָ��λ��, ������ (1)�����޸�״̬����, (2)���Ʊ��޸��ֶ����� (3)��̨�����޸���Ϣ, ͬ���ļ�

	ʵ��
	1 FileDBRecord �б����ļ�λ��
	2 FileDBSource �̳� DataSource, ��ʼʱ, ͳ�Ƴ��ֶ������ļ�ƫ����Ϣ
	3 д��¼ʱ, ������Ҫ�޸ĵ��ֶ��еõ��ļ�λ��ƫ����, ��¼��������(�ļ�λ��ƫ����, �ڻ����е�ƫ����, ���ݴ�С)
struct DataInfo
{
	size_t mFileSeekPos;
	DSIZE mBufferPos;
	DSIZE mDataSize;
}
Array<DataInfo>    mUpdateFieldInfo;
DataBuffer                mUpdateFieldData;

4 ��̨�����ṩ������д�뵽�ļ�ӳ��

	5 �½������¼ʱ, (1)����ļ���С, ���С����Ҫ�ĳߴ�, �������ļ�, ���ؽ��ڴ�ӳ�� (2) д���¼�¼����

	6 ��ȡ��¼, ��ȡ��, ���ļ�λ��ƫ�Ʊ��浽��¼��

	int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hFile = CreateFile("e:/TestFile.bin", GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);


	HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 2, 0, NULL);

	int error = GetLastError();

	PBYTE pbFile = (PBYTE)MapViewOfFile(hFileMapping, SECTION_MAP_READ|SECTION_MAP_WRITE, 0, 0, 0);
	error = GetLastError();

	void *p = ::malloc(0xFFFFFFFF);
	memset(p, 0xFE, 0xFFFFFFFF);
	UINT t = ::GetTickCount();
	for (int i=0; i<100000; ++i)
	{
		memcpy(pbFile, p, 1024);
		pbFile += 8;
	}

	printf("%u", ::GetTickCount()-t);
	return 0;
}

//-------------------------------------------------------------------------*/
#ifndef _INCLUDE_LOCALFILEDBSAVER_H_
#define _INCLUDE_LOCALFILEDBSAVER_H_

#include "DBSaver.h"
#include "MemoryDBHead.h"
#include "FileDataStream.h"
#include "EasyMap.h"
#include "BaseTable.h"
#include "Array.h"
#include "DBCallBack.h"
//-------------------------------------------------------------------------
#define  DB_FILE_PATH	"DB/"

struct _SaveRecordData
{
	bool		mbUpdate;
	DataBuffer	mData;
};
typedef Array<_SaveRecordData>	RecordData;
//-------------------------------------------------------------------------
// ����Ϊ.db�ļ�
// �ļ��ײ�Ϊ����ֶ���Ϣ����¼���ݿ�ʼλ�ã�ɾ���ļ�¼��һ��λ�ã��ڶ���λ�ñ�����ǰһ���ļ�¼λ�ô�
// 
//-------------------------------------------------------------------------
class MemoryDB_Export  LocalFileDBSaver : public tDBSaver
{
public:
	virtual const char* GetDBName() { return mDBName.c_str(); }
	virtual bool Start(NiceData &initParam);

	virtual void Log(const char *szInfo, ...);

	virtual bool InitReadyNewDB(const char *szDBName, tBaseTable *memoryTable, AString tableData);

public:
	virtual bool SaveData(Int64 keyID, void *pData, DSIZE size)
	{
		return SaveData(STRING(keyID), pData, size);
	}
	virtual bool SaveData(const char *szKeyID, void *pData, DSIZE size);

	virtual bool LoadData(Int64 keyID, DataStream *destData)
	{
		return LoadData(STRING(keyID), destData);
	}
	virtual bool LoadData(const char *szKeyID, DataStream *destData);

	// ����MemoryDB ��ʼ�����������ݼ�¼
	virtual bool LoadAllData(LoadDataCallBack &loadCallBack, AString &errorInfo);

	virtual bool DelectData(const char *szKeyID, UInt64 extValue) override;

public:
	virtual bool SaveRecordData(UInt64 &recordPos, const char *szKey, RecordData *data, bool bNewIndex);

	virtual bool ReadAllRecord(tBaseTable *destTable);

	virtual void ResaveRecordCount()
	{
		mDBFile.seek64(sizeof(UInt64));
		mDBFile.write(mRecordCount);
	}

	UInt64 _recordFilePos(UInt64 recordPos)
	{
		return mRecordPos + recordPos * mDBRecordLength;
	}

	bool ReadRecordData(int recordPos, AutoRecord destRecord);

	virtual void OnLoadRecord(int dbPos, AutoRecord re){}

	// ���������ں�̨�߳�
	virtual void OnSaveRecord(int dbPos, const char *szKey)
	{
	}

	virtual void OnDeleteRecord(int dbPos, const char *szKey)
	{
	}

public:
	LocalFileDBSaver()
		: mRecordCount(0)
		, mDeletePos(0)
		, mRecordPos(-1)
		, mDBRecordLength(0)
	{

	}

public:
	AString			mDBName;		// ����Ӧ�ı������
	FileDataStream	mDBFile;

	UInt64			mRecordPos;
	int				mRecordCount;	
	int				mDeletePos;

	AutoField		mTableField;
	int				mDBRecordLength;
};
//-------------------------------------------------------------------------
// ���ڶ����첽����ɾ����LocalDB�洢
class ThreadLocalFileDBSaver : public LocalFileDBSaver
{
public:
	virtual bool Start(NiceData &initParam) override
	{
		if (LocalFileDBSaver::Start(initParam))
		{
			mbStringKey = mTableField->getFieldInfo(0)->getType()==FIELD_STRING;
			return true;
		}
		return false;
	}

	virtual bool SaveRecordData(UInt64 &recordPos, const char *szKeyID, RecordData *data, bool bNewIndex) override
	{
		UInt64 pos = 0;
		if (!bNewIndex)
		{
			if (mbStringKey)
			{
				int *p = mStringDBPosIndex.findPtr(szKeyID);
				if (p==NULL)
				{
					ERROR_LOG("[%d] db key is not exist");
					return false;
				}
				pos = *p;
			}
			else
			{
				int *p = mDBPosIndex.findPtr((Int64)TOUINT64(szKeyID));
				if (p==NULL)
				{
					ERROR_LOG("[%d] db key is not exist");
					return false;
				}
				pos = *p;
			}
		}
		return LocalFileDBSaver::SaveRecordData(pos, szKeyID, data, bNewIndex);
	}

	virtual bool DelectData(const char *szKeyID, UInt64 extValue) override
	{
		if (mbStringKey)
		{
			int *p = mStringDBPosIndex.findPtr(szKeyID);
			if (p==NULL)
			{
				ERROR_LOG("[%d] db key is not exist");
				return false;
			}
			extValue = *p;
		}
		else
		{
			int *p = mDBPosIndex.findPtr((Int64)TOUINT64(szKeyID));
			if (p==NULL)
			{
				ERROR_LOG("[%d] db key is not exist");
				return false;
			}
			extValue = *p;
		}
		return LocalFileDBSaver::DelectData(szKeyID, extValue);
	}

	virtual void OnLoadRecord(int dbPos, AutoRecord re) override
	{
		if (mbStringKey)
		{
			AString key = re[0].string();
			if (mStringDBPosIndex.exist(key))
			{
				ERROR_LOG("���ش���, [%s] DB�����ظ���KEY [%s]", mDBName.c_str(), key.c_str());
			}
			mStringDBPosIndex.insert(key, dbPos);
		}
		else
		{
			Int64 key = (Int64)(UInt64)re[0];
			if (mDBPosIndex.exist(key))
			{
				ERROR_LOG("���ش���, [%s] DB�����ظ���KEY [%llu]", mDBName.c_str(), key);
			}
			mDBPosIndex.insert(key, dbPos);
		}
	}

	virtual void OnSaveRecord(int dbPos, const char *szKey)
	{
		if (mbStringKey)
		{
			if (mStringDBPosIndex.exist(szKey))
			{
				WARN_LOG("NOTE: [%s] DB�����ظ���KEY [%s]", mDBName.c_str(), szKey);
			}
			mStringDBPosIndex.insert(szKey, dbPos);
		}
		else
		{
			Int64 key = TOUINT64(szKey);
			if (mDBPosIndex.exist(key))
			{
				WARN_LOG("NOTE: [%s] DB�����ظ���KEY [%s]", mDBName.c_str(), szKey);
			}
			mDBPosIndex.insert(key, dbPos);
		}
	}

	virtual void OnDeleteRecord(int dbPos, const char *szKey)
	{
		bool bRemove = false;
		if (mbStringKey)
		{
			bRemove = mStringDBPosIndex.erase(szKey);
		}
		else
		{
			Int64 key = TOUINT64(szKey);			
			bRemove = mDBPosIndex.erase(key);
		}
		if (!bRemove)
		{
			ERROR_LOG(" [%s] DBKEY�Ƴ�ʧ��, ������ KEY [%s]", mDBName.c_str(), szKey);
		}
	}

public:
	ThreadLocalFileDBSaver()
		: mbStringKey(false){}

public:
	EasyHash<Int64, int>	mDBPosIndex;
	EasyHash<AString, int>	mStringDBPosIndex;

	bool					mbStringKey;
};

//-------------------------------------------------------------------------

#endif //_INCLUDE_LOCALFILEDBSAVER_H_