
#ifndef _INCLUDE_FIELDSUBTABLE_H_
#define _INCLUDE_FIELDSUBTABLE_H_

#include "BaseCommon.h"
#include "IndexBaseTable.h"
#include <set>
#include <string>

//-------------------------------------------------------------------------
// 表格中的子表对象
// 理论上占用空间越小越好,每条记录中的此表会实例一个对象
// NOTE:	此表最好覆盖 NewRecordIndex 函数, 返回最小空间的记录索引, 
//				当前字符索引使用 EasyHash, 可考虑使用EasyMap
//-------------------------------------------------------------------------
class BaseCommon_Export FieldSubTable : public NormalBaseTable
{
public:
	FieldSubTable(INIT_FIELD_MODE initMode)
		: NormalBaseTable(initMode)
	{
	}
	virtual const char* GetTableType(){ return "FieldTable"; }
	virtual const char* GetTableName() const { return ""; }
	virtual void SetTableName(const char *szTableName)
	{
		//if (szTableName!=NULL && strlen(szTableName)>0)
		//	WARN_LOG("不能保存表格索引名称[%s],子表为了节省空间", szTableName); 
	}

public:
	virtual bool HasDeleteRecord(){ return !mDelectKeyList.empty(); }
	virtual void ClearDeleteRecordList(){ mDelectKeyList.clear(); }

public:
	bool AppendRecord( ARecord scrRecord, bool bReplace, bool bRestore = false ) override;

	bool RemoveRecord( float fIndex )
	{
		if ( BaseTable::RemoveRecord(fIndex) )	
		{
			mDelectKeyList.insert(STRING(fIndex));
			return true;
		}
		return false;
	}

	bool RemoveRecord( const char* szIndex )
	{		
		if (BaseTable::RemoveRecord(szIndex))
		{
			mDelectKeyList.insert(szIndex);
			return true;
		}
		return false;
	}

	bool RemoveRecord( Int64 nIndex )
	{
		if (BaseTable::RemoveRecord(nIndex))
		{
			mDelectKeyList.insert(STRING(nIndex));
			return true;
		}
		return false;
	}

	bool RemoveRecord( ARecord record )
	{
		if (BaseTable::RemoveRecord(record))
		{
			mDelectKeyList.insert(record->getIndexData().string());
			return true;
		}
		return false;
	}

	virtual bool DeleteRecord(ARecord record)
	{
		return RemoveRecord(record);
	}

	virtual bool SaveDeleteInfo(DataStream *resultStream, bool bClearDeleteList)
	{
		resultStream->write((int)mDelectKeyList.size());
		for (std::set<AString>::iterator it=mDelectKeyList.begin(); it!=mDelectKeyList.end(); ++it)
		{
			resultStream->writeString((*it).c_str());
		}
		if (bClearDeleteList)
			mDelectKeyList.clear();
		return true;
	}

	virtual bool ApplyDeleteInfo(DataStream *scrData)
	{
		int count = 0;
		READ(scrData, count);

		for (size_t i=0; i<count; ++i)
		{
			EasyString key;
			if (!scrData->readString(key))
			{
				ERROR_LOG("恢复删除记录KEY失败");
				return false;
			}
			RemoveRecord(key.c_str());
		}
		return true;
	}

	virtual void ClearAll()
	{
		if (!mMainKeyIndex)
			return;

		for(ARecordIt it=GetRecordIt(); *it; ++(*it))
		{
			ARecord r = *it;
			if (r)
				mDelectKeyList.insert(r->getIndexData().string());
		}
		BaseTable::ClearAll();
	}


protected:
	std::set<AString>	mDelectKeyList;
};


#endif //_INCLUDE_FIELDTABLE_H_