
#include "LocalFileDBSaver.h"
#include "TableTool.h"
#include "NiceData.h"
#include <stdarg.h>
#include <stdio.h>
#include "IndexBaseTable.h"
#include "MemoryDBRecord.h"

bool LocalFileDBSaver::Start( NiceData &initParam )
{
	mDBName = initParam.get(DBNAME).string();
	AssertEx(!mDBName.empty(), "DB名称为空, 需要提供参数 [DBNAME]");
	AString fileName = DB_FILE_PATH;
	fileName += mDBName.c_str();
	fileName += ".mdb";

	if (!mDBFile.open(fileName.c_str(), FILE_READ_WRITE))
	{
		ERROR_LOG("打开DB数据文件错误 [%s]", fileName.c_str());
		return false;
	}

	mDBFile.read(mRecordPos);
	mDBFile.read(mRecordCount);
	mDBFile.read(mDeletePos);

	int code = 0;
	mDBFile.read(code);

	DataBuffer d(1024);
	AString f;
	if (!mDBFile.readData(&d) || !mDBFile.readString(f))
	{
		ERROR_LOG("Read skip data or read field string data  fail");
		return false;
	}
	
	mTableField = MEM_NEW BaseFieldIndex();
	if (!mTableField->FullFromString(f))
	{
		ERROR_LOG("[%s] DB table field restore fail", mDBName);
		return false;
	}

	if (mTableField->GetCheckCode()!=code)
	{
		ERROR_LOG("[%s] DB table field check fail", mDBName);
		return false;
	}

	mDBRecordLength = 1;
	for (int i=0; i<mTableField->getCount(); ++i)
	{
		mDBRecordLength += mTableField->getFieldInfo(i)->getDBDataLength();	
	}

	// 计算出记录总数
	//UInt64 t = mDBFile.size64();
	//mRecordCount = (int)((t-mRecordPos)/mDBRecordLength);
	NOTE_LOG("[%s] total alloct record >[%d]", mDBName.c_str(), mRecordCount-1);
	return true;
}

void LocalFileDBSaver::Log( const char *szInfo, ... )
{
#if DEVELOP_MODE
	va_list va;
	va_start(va, szInfo);
	LOG_TOOL(va, szInfo);
#endif
}

bool LocalFileDBSaver::InitReadyNewDB( const char *szDBName, tBaseTable *memoryTable, AString tableData )
{
	//警告 会覆盖掉原来的数据
	//mDBName = pInitParam.get(DBNAME).c_str();
	AssertEx(szDBName!=NULL, "DB名称为空, 需要提供参数 [DBNAME]");
	AString fileName = DB_FILE_PATH;
	fileName += szDBName;
	fileName += ".mdb";		

	FileDataStream dbFile;
	if (!dbFile.open(fileName.c_str(), FILE_CREATE))
	{
		ERROR_LOG("创建DB数据文件错误 [%s]", fileName.c_str());
		return false;
	}

	mDeletePos = 0;
	mRecordCount = 1;	// 默认从1开始
	
	// 保存表格字段，及初始记录开始位置，删除第一个的位置
	int code  = memoryTable->GetField()->GetCheckCode();
	AString fieldData = memoryTable->GetField()->ToString();
	dbFile.write((UInt64)0);						// 记录在文件中开始的位置
	dbFile.write((int)mRecordCount);		// 当前记录数量, NOTE: 记录位置从1开始
	dbFile.write((int)mDeletePos);				// 第一个删除的记录位置
	dbFile.write(code);								// 字段验证code
	DataBuffer t(1024);						// 10M保留空间
	dbFile.writeData(&t, 1024);					
	dbFile.writeString(fieldData.c_str());		// 表格字段信息
	mRecordPos= dbFile.tell();					// 记录开始位置

	dbFile.seek(0);
	dbFile.write(mRecordPos);

	dbFile.close();

	return true;
}

bool LocalFileDBSaver::SaveData( const char *szKeyID, void *pData, DSIZE size )
{
	ERROR_LOG("LocalFileDBSaver not use SaveData");
	return false;
}

bool LocalFileDBSaver::LoadData( const char *szKeyID, DataStream *destData )
{
	ERROR_LOG("LocalFileDBSaver not use LoadData");
	return false;
}

bool LocalFileDBSaver::LoadAllData( LoadDataCallBack &loadCallBack, AString &errorInfo )
{
	return false;
}

bool LocalFileDBSaver::DelectData( const char *szKeyID, UInt64 extValue )
{
	// 将extValue位置设置为删除标识
	UInt64 pos = _recordFilePos(extValue);
	if (!mDBFile.seek64(pos))
	{
		ERROR_LOG("[%s] db Seek to [%llu] fail", mDBName.c_str(), pos);
		return false;
	}
	mDBFile.write(true);
	// 写入最后删除的记录位置, 形成一个单向链
	mDBFile.write(mDeletePos);
	
	mDeletePos = (int)extValue;
	// 改变文件中的最后删除的
	if (mDBFile.seek64(sizeof(UInt64)+sizeof(int)) )
	{
		mDBFile.write(mDeletePos);
		mDBFile.flush();
		OnDeleteRecord((int)extValue, szKeyID);
		return true;
	}
	else
		ERROR_LOG("[%s] db Seek to [%llu] fail", mDBName.c_str(), sizeof(UInt64)+sizeof(int));

	mDBFile.flush();

	return false;
}



bool LocalFileDBSaver::SaveRecordData(UInt64 &recordDBPos, const char *szKey, RecordData *data, bool bNewIndex)
{	
	bool bNew = false;
	if (bNewIndex && recordDBPos>0)
	{
		WARN_LOG("插入记录, DB pos 必须为 0, 当前转为更新保存");
        bNewIndex = false;
		//return false;
	}
	if (recordDBPos<=0 && bNewIndex)
	{	
		// 先检查使用删除的位置
		if (mDeletePos>0)
		{
			recordDBPos = mDeletePos;
			// 读取上一个删除的位置
			if (!mDBFile.seek64(_recordFilePos(mDeletePos)+1))
			{
				ERROR_LOG("[%s] DB seek fail >[%llu]", mDBName.c_str(), _recordFilePos(mDeletePos)+1);
				return false;
			}
			int deletePos = 0;
			if (!mDBFile.read(deletePos))
			{
				ERROR_LOG("[%s] DB read last delete pos fail >[%llu]", mDBName.c_str(), _recordFilePos(mDeletePos)+1);
				return false;
			}
			mDeletePos = deletePos;
			// 重写删除位置
			if (!mDBFile.seek64(sizeof(UInt64)+sizeof(int)) )
			{
				ERROR_LOG("[%s] DB seek delete pos fail >[%llu]", mDBName.c_str(), sizeof(UInt64)+sizeof(int));
				return false;
			}
			mDBFile.write(mDeletePos);	

			// 清除删除标签
			mDBFile.seek64(_recordFilePos(recordDBPos));
			mDBFile.write(false);
		}
		else
		{		
			bNew = true;
			recordDBPos = mRecordCount;
		}
	}
	// 计算记录在文件中的位置
	UInt64 pos = mRecordPos + recordDBPos * mDBRecordLength;
	UInt64 recordEndPos = pos + mDBRecordLength;

	pos += 1;    // 偏移1个字段标签

	for (int i=0; i<data->size(); ++i)
	{
		_SaveRecordData &d  = (*data)[i];
		if (d.mbUpdate)
		{
			if (!mDBFile.seek64(pos)) 
			{
				ERROR_LOG("[%s] db File write error", mDBName.c_str());
				return false;
			}
			int len = mTableField->getFieldInfo(i)->getDBDataLength();
			if (d.mData.dataSize()<len)
				len = d.mData.dataSize();

			if (!mDBFile._write(d.mData.data(), len))
			{
				ERROR_LOG("[%s] Write db file record [%llu] data fail", mDBName.c_str(), recordDBPos);
				return false;
			}
		}
		pos += mTableField->getFieldInfo(i)->getDBDataLength();
		if (pos>recordEndPos)
		{
			ERROR_LOG("[%s] Record data pos more then RecordLength", mDBName.c_str());
			return false;
		}
	}

	if (bNew)
	{
		++mRecordCount;
		ResaveRecordCount();
	}

	if (bNewIndex)
		OnSaveRecord((int)recordDBPos, szKey);

	return true;
}

bool LocalFileDBSaver::ReadAllRecord(tBaseTable *destTable)
{
	//DataBuffer fieldData(1024);
	//mTableField->saveToData(&fieldData);
	//fieldData.seek(0);
	//destTable->GetField()->restoreFromData(&fieldData);
	//destTable->SetIDMainIndex()
	if (destTable->GetField()->GetCheckCode() != mTableField->GetCheckCode())
	{
		ERROR_LOG("[%s] DB table field is not same of DB file", mDBName.c_str());
		return false;
	}

	int fieldCount = mTableField->getCount();
	const FieldIndex::FieldVec &fV = mTableField->getFieldInfoList();

	for (int i=1; i<mRecordCount; ++i)
	{
		UInt64 pos = mRecordPos + i * mDBRecordLength;
		if (!mDBFile.seek64(pos))
		{
			ERROR_LOG("Read db file error >[%s]", mDBName.c_str());
			return false;
		}

		bool bDelete = false;
		if (!mDBFile.read(bDelete))
		{
			ERROR_LOG("Read bDelete from db file error >[%s]", mDBName.c_str());
			break;
			//return false;
		}
		if (bDelete)
			continue;

		AutoRecord re = destTable->NewRecord();
		MemoryDBRecord *p = dynamic_cast<MemoryDBRecord*>(re.getPtr());
		pos += 1;
		for (int col=0; col<fieldCount; ++col)
		{
			if (!mDBFile.seek64(pos))
			{
				ERROR_LOG("Read db file error >[%s]", mDBName.c_str());
				return false;
			}
			FieldInfo info = fV[col];
			if (!info->restoreData(p->_getRecordData(), &mDBFile))
			{
				ERROR_LOG("[%s] Restore record data fail, col >[%d]", mDBName.c_str(), col);
				return false;
			}
			pos += info->getDBDataLength();
		}
		//NOTE: 不能使用记录的直接恢复, 因为读取表格字段时，不会全部读取 16K字节
		//if (!re->restoreData(&mDBFile))
		//{
		//	ERROR_LOG("[%s] DB record resotre faile", mDBName.c_str());
		//	return false;
		//}
		p->mExtValue = i;
		OnLoadRecord(i, re);
		destTable->AppendRecord(re, true);
		re->FullAllUpdate(false);
	}
#if DEVELOP_MODE
	AString fName;
	fName.Format("TableLog/%s.txt", mDBName.c_str(), true);
	destTable->SaveCSV(fName.c_str());
#endif
	return true;
}

bool LocalFileDBSaver::ReadRecordData(int recordPos, AutoRecord destRecord)
{
	UInt64 pos = _recordFilePos(recordPos);
	if (!mDBFile.seek64(pos))
	{
		ERROR_LOG("Read db file error >[%s]", mDBName.c_str());
		return false;
	}

	bool bDelete = false;
	if (!mDBFile.read(bDelete))
	{
		ERROR_LOG("Read bDelete from db file error >[%s]", mDBName.c_str());
		return false;
		//return false;
	}
	if (bDelete)
	{
		ERROR_LOG("[%s] db record pos [%d] is delete", mDBName.c_str(), recordPos);
		return false;
	}

	MemoryDBRecord *p = dynamic_cast<MemoryDBRecord*>(destRecord.getPtr());
	pos += 1;
	AutoField f = destRecord->getField();
	for (int col=0; col<f->getCount(); ++col)
	{
		if (!mDBFile.seek64(pos))
		{
			ERROR_LOG("Read db file error >[%s]", mDBName.c_str());
			return false;
		}
		FieldInfo info = f->getFieldInfo(col);
		if (!info->restoreData(p->_getRecordData(), &mDBFile))
		{
			ERROR_LOG("[%s] Restore record data fail, col >[%d]", mDBName.c_str(), col);
			return false;
		}
		pos += info->getDBDataLength();
	}
	//NOTE: 不能使用记录的直接恢复, 因为读取表格字段时，不会全部读取 16K字节
	//if (!re->restoreData(&mDBFile))
	//{
	//	ERROR_LOG("[%s] DB record resotre faile", mDBName.c_str());
	//	return false;
	//}
	p->mExtValue = recordPos;
	p->FullAllUpdate(false);

	return true;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

