/********************************************************************
	created:	2011/07/10
	created:	10:7:2011   0:39
	filename: 	d:\NewTable__0709\NewTable\Common\DataBase\FieldIndex.h
	file path:	d:\NewTable__0709\NewTable\Common\DataBase
	file base:	FieldIndex
	file ext:	h
	author:		Yang Wenge
	
	purpose:	特别注意: 此对象不可重用修改, 原因是当修改后,使用到此对象的记录释放时, 有可能会出现清理内存泄漏
*********************************************************************/

#ifndef _INCLUDE_FIELDINDEX_H_
#define _INCLUDE_FIELDINDEX_H_

#include "FieldInfo.h"
#include "Array.h"
#include "EasyMap.h"
#include "EasyHash.h"

#include "AutoPtr.h"

#define FIELD_USE_HASH_INDEX		1				// 使用快速HASH


#define FIELD_MAX_COUNT				256
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
class tBaseTable;
class DataStream;
class tBaseTable;
class BaseRecord;
//------------------------------------------------------------------------------------
//字段索引结构
class BaseCommon_Export FieldIndex : public AutoBase
{
	friend class BaseRecord;
public:
	typedef Array<FieldInfo>				FieldVec;

#if FIELD_USE_HASH_INDEX
	typedef EasyHash<AString, short, 4>		FieldMap;
#else
	typedef EasyMap<EasyString, short>		FieldMap;
#endif

private:
	FieldVec	mFieldVec;
	FieldMap	mFieldMap;
	size_t		mDataLength;

public:
	// 默认为1，表示更新状态，0表示不更新，与状态相与计算 NOTE: 此状态不参加复制
	Buffer		mUpdateState;			

public:
	void _appendField(const char* fieldName, int col);

	bool _replaceField(int col, FieldInfo newField);

	// 优化字段名列表,可提高查询速度 
	void _optimizeNameHash(float rate)
	{ 
#if FIELD_USE_HASH_INDEX
		mFieldMap.OptimizeSpeed(rate);
#endif
	}

	virtual void* _allotRecordData(uint size) { return  ALLOCT_NEW(size); }
	virtual void _freeRecordData(void *pRecordData) {ALLOCT_FREE(pRecordData); }

	virtual BaseRecord* TryGetFreeRecord() { return NULL; }

public:
	friend class DataRecord;
	friend class tBaseTable;

	FieldIndex( tBaseTable *pTable, size_t num );
	~FieldIndex();

public:
	int getCount() const { return (int)mFieldVec.size(); }

	const FieldVec& getFieldInfoList() const { return mFieldVec; }
	const FieldMap&	getFieldMap() const { return mFieldMap; }

	//得到一个字段在索引中的位置(列),如果不存在则返回-1;
	FieldInfo getFieldInfo( const char *name ) const;
	FieldInfo getFieldInfo( int col ) const;

	int getFieldCol( const char *name ) const;
	bool existField( const char *name ) const;

	int getFieldType(int col);

	bool operator ==( const FieldIndex &other ) const;
	FieldInfo operator [] (int col) const
	{
		if (col>=0 && col<(int)mFieldVec.size())
			return mFieldVec[col];

		return NULL;
	}

	//检查向量与索引是否一一对应
	bool check() const;

	// 用于DB表格
	virtual bool needCheck() const { return false; }
	virtual bool CheckSame(int checkCode) const { return true; }
	virtual int GetCheckCode() const { return 0; }

	//复制
	FieldIndex& operator = ( const FieldIndex& other );

	// 检测字段名是否符合要求
	bool checkFieldName(const AString &fieldNameList) const;

	size_t getDataLength(void) const
	{
		return mDataLength;
	}

	bool saveToData(DataStream *destData) const;
	bool restoreFromData(DataStream *scrData);

public:
	//设置字段
	FieldInfo setField( const char *name, int type, int col );
	FieldInfo setField( const char *name, const char* strType, int col );

	//删除最后一个字段信息,一般说,如果删除中间字段信息,需要进行整理,或者把后面的字段信息全向前移动
	void deleBackField( );

	virtual void _updateInfo();

	// for Redis save load
	AString ToString() const;

	bool FullFromString(const AString &scrString);

	virtual bool SetSkipUpdate(int nFieldCol)
	{ 
		return mUpdateState.closeState(nFieldCol);		
	}
	virtual bool IsSkipUpdate(int nFieldCol){ return !mUpdateState.isOpenState(nFieldCol); }

protected:
	virtual void FreeRecord(BaseRecord *pRecord);

public:
	static FieldInfo createFiledInfo(const char* strType);
	static FieldInfo createFiledInfo(int type);
	static FieldInfo createFieldInfo(int type);
	static FieldInfo createFieldInfo(const char* typeName);
};
//------------------------------------------------------------------------------------
typedef Auto<FieldIndex>	AutoField;

//------------------------------------------------------------------------------------


#endif //_INCLUDE_FIELDINDEX_H_