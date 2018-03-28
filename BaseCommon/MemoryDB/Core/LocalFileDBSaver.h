/*/-------------------------------------------------------------------------
	DB本地落地优化方案 (可简化数据源存储过程, 且支持数据即时修改同步, 理论上不需要再设置写入限制, 全部提交给系统同步, 安全系统较高)
	1 如下代码测试, 使用内存文件映射后, 频繁写入修改小块内存时, 速度非常快(100000 次=15毫秒)
	2 实现FileDBRecord继承MemoryDBRecord 保存文件内存指针, 实现FileDBTable继承MemoryDBTable, 使用FileDBRecord, 创建后, 设置文件指针
	3 在记录需要修改时, 对修改的字段进行文件指针定位 (记录文件指针+4字节字段校验+前面字段getDBDataLength()), FileDBTable有必要使用专用字段, 用来保存字段偏移指针位置, STRING = 32, TABLE = 64K
	4 字段数据直接memcpy至映射位置
	5 数据表格结构保存为文本方式(CSV), 方便设定表格分布
	6 如果使用后台的话, 可创建规定数量任务数, 任务中分配文件中记录需要的长度缓存, 任务中设置记录的文件指针位置, 缓存中 (1)复制修改状态数据, (2)复制被修改字段数据 (3)后台根据修改信息, 同步文件

	实现
	1 FileDBRecord 中保存文件位置
	2 FileDBSource 继承 DataSource, 初始时, 统计出字段数据文件偏移信息
	3 写记录时, 根据需要修改的字段列得到文件位置偏移量, 记录到数组中(文件位置偏移量, 在缓存中的偏移量, 数据大小)
struct DataInfo
{
	size_t mFileSeekPos;
	DSIZE mBufferPos;
	DSIZE mDataSize;
}
Array<DataInfo>    mUpdateFieldInfo;
DataBuffer                mUpdateFieldData;

4 后台根据提供的数据写入到文件映射

	5 新建插入记录时, (1)检查文件大小, 如果小于需要的尺寸, 先增长文件, 再重建内存映射 (2) 写入新记录数据

	6 读取记录, 读取后, 把文件位置偏移保存到记录中

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
// 保存为.db文件
// 文件首部为表格字段信息，记录数据开始位置，删除的记录第一个位置，第二个位置保存在前一个的记录位置处
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

	// 用于MemoryDB 初始加载所有数据记录
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

	// 以下运行在后台线程
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
	AString			mDBName;		// 即对应的表格名称
	FileDataStream	mDBFile;

	UInt64			mRecordPos;
	int				mRecordCount;	
	int				mDeletePos;

	AutoField		mTableField;
	int				mDBRecordLength;
};
//-------------------------------------------------------------------------
// 用于多线异步保存删除的LocalDB存储
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
				ERROR_LOG("严重错误, [%s] DB存在重复的KEY [%s]", mDBName.c_str(), key.c_str());
			}
			mStringDBPosIndex.insert(key, dbPos);
		}
		else
		{
			Int64 key = (Int64)(UInt64)re[0];
			if (mDBPosIndex.exist(key))
			{
				ERROR_LOG("严重错误, [%s] DB存在重复的KEY [%llu]", mDBName.c_str(), key);
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
				WARN_LOG("NOTE: [%s] DB存在重复的KEY [%s]", mDBName.c_str(), szKey);
			}
			mStringDBPosIndex.insert(szKey, dbPos);
		}
		else
		{
			Int64 key = TOUINT64(szKey);
			if (mDBPosIndex.exist(key))
			{
				WARN_LOG("NOTE: [%s] DB存在重复的KEY [%s]", mDBName.c_str(), szKey);
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
			ERROR_LOG(" [%s] DBKEY移除失败, 不存在 KEY [%s]", mDBName.c_str(), szKey);
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