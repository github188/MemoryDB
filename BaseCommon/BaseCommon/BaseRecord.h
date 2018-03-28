
#ifndef _INCLUDE_BASERECORD_H_
#define _INCLUDE_BASERECORD_H_

#include "Hand.h"
#include "FieldIndex.h"
#include "Data.h"
#include "BaseCommon.h"
#include "NiceData.h"
#include "EasySet.h"
#include "RecordTool.h"
//-------------------------------------------------------------------------
class BaseCommon_Export  BaseRecord : public AutoBase  // Base<BaseRecord>
{
	friend class BaseTable;
	friend class MemoryDBTable;
	friend class MemoryDBIndexTable;
	friend struct Data;

public:
	Data getIndexData();
	Data get(const AString &fieldName) const { return get(getFieldCol(fieldName.c_str())); }
	Data get(const char* szField) const { return get(getFieldCol(szField)); }
	Data getData(const char* szField)const{ return get(getFieldCol(szField)); }
	Data getData(int col) const { return get(col); }

	int getInt(const char* szField) const { int r = 0; FieldInfo info = getFieldInfo(getFieldCol(szField)); if (info!=NULL) info->get(_getData(), r); return r; }
	float getFloat(const char* szField) const { float r = 0; FieldInfo info = getFieldInfo(getFieldCol(szField)); if (info!=NULL) info->get(_getData(), r); return r; }
	// WARN: Return char value only temp use, example in lua scrip.
	const char* getString(const char* szField) const;

	// for Hand [] operate
	virtual Data _getData(const char *szKey) const override { 	return get(szKey); }
	virtual Data _getData(int field) const override { return get(field); }

	tBaseTable* getTable(const char* szField) const{ return getTable(getFieldCol(szField));}
	tBaseTable* getTable(int col)const;
	AutoNice getNice(const char* szField) const{ return getNice(getFieldCol(szField));}
	AutoNice getNice(int col)const;
	AutoData getDataBuffer(const char* szField) const{ return getDataBuffer(getFieldCol(szField));}
	AutoData getDataBuffer(int col)const;

	//Hand<BaseRecord> getRecord(const char* szField) const{ return getRecord(getFieldCol(szField));}
	//Hand<BaseRecord> getRecord(int col) const;
	
	bool get(const char* szField, AutoData &resultData )const{ return get(getFieldCol(szField), resultData); }
	bool get(int col, AutoData &resultData) const {  return get(col, &resultData, typeid(resultData)); }

	bool get(const char* szField, Hand<BaseRecord> &resultRecord )const{ return get(getFieldCol(szField), resultRecord); }
	bool get(int col, Hand<BaseRecord> &resultRecord) const {  return get(col, &resultRecord, typeid(resultRecord)); }


	bool get(const char *szField, void *resultValue, const type_info &typeInfo)const{ return get(getFieldCol(szField), resultValue, typeInfo); }
	bool get(int col, void *resultValue, const type_info &typeInfo) const;

public:
	bool set(const char* szField, int nVal){ return set (getFieldCol(szField), nVal); }
	bool set(const char* szField, Int64 nVal){ return set (getFieldCol(szField), nVal); }
	bool set(const char* szField, UInt64 nVal){ return set (getFieldCol(szField), (Int64)nVal); }
	bool set(const char* szField, float nVal){ return set (getFieldCol(szField), nVal); }
	bool set(const char* szField, const char *szVal){ return set (getFieldCol(szField), szVal); }
	bool set(const char* szField, const AString &strVal){ return set(getFieldCol(szField), strVal); }
	bool set(const char* szField, void *obj, const type_info &typeInfo){ return set(getFieldCol(szField), obj, typeInfo); }	
	bool set(const char* szField, AutoData data){ return set(getFieldCol(szField), data); }

	bool set(const AString &key, int val){ return set(key.c_str(), val); }
	bool set(const AString &key, Int64 val){ return set(key.c_str(), val); }
	bool set(const AString &key, UInt64 val){ return set(key.c_str(), val); }
	bool set(const AString &key, float val){ return set(key.c_str(), val); }
	bool set(const AString &key, const char *val){ return set(key.c_str(), val); }
	bool set(const AString &key, const AString &val){ return set(key.c_str(), val); }
	bool set(const AString &key, AutoData val){ return set(key.c_str(), val); }
	bool set(const AString &key, void *obj, const type_info &typeInfo){ return set(key.c_str(), obj, typeInfo); }	

	bool set(int col, AutoData data){ return set(col, &data, typeid(data)); }

	bool set(const char* szField, AutoNice data){ return set(getFieldCol(szField), data); }
	bool set(int col, AutoNice data){ return set(col, &data, typeid(data)); }

	bool set(const char* szField, tBaseTable *data){ return set(getFieldCol(szField), data); }
	bool set(int col, tBaseTable *data);

	bool set(const char *szField, const Data &scrData){ return set (getFieldCol(szField), scrData); }
	bool set(int col,  const Data &scrData);

	bool set(const AString &key, AutoNice val){ return set(key.c_str(), val); }
	bool set(const AString &key, tBaseTable *val){ return set(key.c_str(), val); }
	bool set(const AString &key, const Data &val){ return set(key.c_str(), val); }
	//bool set(const char* szField, Hand<BaseRecord> data) { return set(getFieldCol(szField), data); }
	//bool set(int col,  Hand<BaseRecord> data) { return set(col, &data, typeid(data)); }

public:
	virtual bool set(int col, int nVal){ return set(col, (Int64)nVal); }
	virtual bool set(int col, Int64 nVal){ return _set (col, nVal); }
	virtual bool set(int col, float nVal){ return _set (col, nVal); }
	virtual bool set(int col, const char *szVal){ return _set(col, szVal); }
	virtual bool set(int col, const AString &strVal){ return _set(col, strVal); }
	virtual bool set(int col, void *obj, const type_info &typeInfo){ return _set(col, obj, typeInfo); }

public:
	virtual bool _set(int col, Int64 nVal);
	virtual bool _set(int col, float nVal);
	virtual bool _set(int col, const char *szVal);
	virtual bool _set(int col, const AString &strVal);
	virtual bool _set(int col, void *obj, const type_info &typeInfo);

public:
	BaseRecord()
		: mRecordData(NULL)
	{

	}

	virtual ~BaseRecord()
	{
		_freeData();
	}

	virtual void Release() override;
	Auto<BaseRecord> GetSelf() { return Auto<BaseRecord>(this); }

public:
	virtual void InitData();
	virtual AutoField getField() const = 0;
	int getFieldCol(const char *szField) const;
	FieldInfo getFieldInfo(const char *szField) const { return getFieldInfo(getFieldCol(szField)); }
	FieldInfo getFieldInfo(int col) const { return getField()->getFieldInfo(col); }

public:
	virtual bool FullFromString( const char *scrData, bool bHaveKey, bool bNeedCheck = false );
	virtual bool ToString( AString &resultData, bool bHaveKey, bool bNeedCheck = false );
	virtual AutoNice ToNiceData();
	virtual bool SaveToArrayData(DataStream *pDestData, EasySet<AString> *pSelect = NULL, EasySet<AString> *pExclude = NULL);
	virtual bool SaveSelectData(DataStream *pDestData, EasyMap<int, FieldInfo> &selectField);
	virtual bool restoreSelectData(DataStream *pScrData, EasyMap<int, FieldInfo> &selectField );
    virtual bool SaveSelectData(DataStream *pDestData, Buffer &excludeFieldState);

	virtual bool saveData( DataStream *destData );
	virtual bool restoreData( DataStream *scrData );

	virtual bool saveUpdateData( DataStream *destData ){ return false; }

#if RECORD_ONLY_CALL_RESTOREDATA
protected:
	virtual bool _updateFromData( DataStream *scrData );
public:
	virtual bool updateFromData( DataStream *scrData ){ return restoreData(scrData); }
#else
	virtual bool updateFromData( DataStream *scrData ){ return false; }
#endif
	
	virtual bool InUpdate() const { return false; }
	virtual bool NeedUpdate(void){ return false; }
	virtual bool InTable() const { return true;  }
	virtual void SetInTable(bool bIn){}
	virtual void FullAllUpdate(bool bNeed){}
	virtual bool NotifyChanged(const char *szField){ return NotifyChanged(getFieldCol(szField)); }
	virtual bool NotifyChanged(int col){ return false; }
	virtual bool HadChanged(int col){ return false; }
	virtual void Update(){}

	tBaseTable* CreateFieldTable( const char* szFieldName ){ return CreateFieldTable(getFieldCol(szFieldName)); }
	bool SetFieldTable( const char* szFieldName, tBaseTable *pSubTable ){return SetFieldTable(getFieldCol(szFieldName), pSubTable); }
	tBaseTable* GetFieldTable( const char* szFieldName ){ return GetFieldTable(getFieldCol(szFieldName)); }

	virtual tBaseTable* CreateFieldTable( int col );
	bool SetFieldTable( int col, tBaseTable *pSubTable );
	virtual tBaseTable* GetFieldTable( int col );

	int GetFieldNum(){ return getField()->getCount(); }
	virtual tBaseTable* GetTable() = 0;
	virtual void Remove();

	template<typename T>
	bool GetArray(const char *szField, SaveDBArray<T> &operatorArray);
	template<typename T>
	bool ReadArray(const char *szField, Array<T> &onlyReadArray);

public:
	Data get(int col) const
	{
		Data result;
		if (col<0)
			return result;

		result.mFieldCol = col;
		result.mData = (void*)this;
		return result;
	}
	virtual bool IsNull() const { return false; }
	virtual void _alloctData( int extData );

protected:
	virtual char* _getData() const { return mRecordData; }

	virtual void _freeData()
	{ 
		if (mRecordData!=NULL) 
		{
			ALLOCT_FREE(mRecordData);
			mRecordData = NULL;
		}
	}

	virtual char* _allotMemory(uint size){ return (char*)ALLOCT_NEW(size); }

protected:
	char		*mRecordData;
};
//-------------------------------------------------------------------------
template<typename T>
bool BaseRecord::GetArray(const char *szField, SaveDBArray<T> &operatorArray)
{
	operatorArray.nCol = getFieldCol(szField);
	FieldInfo info = getFieldInfo(operatorArray.nCol);
	if (info != NULL && info->getType() == FIELD_DATA)
	{
		AutoData data = getDataBuffer(operatorArray.nCol);
		bool bRead = true;
		if (data && data->dataSize()>0)
		{
			data->seek(0);
			bRead = data->readArray(operatorArray);
		}
		if (bRead)
			operatorArray.re = this;
		return bRead;
	}
	return false;
}
template<typename T>
bool BaseRecord::ReadArray(const char *szField, Array<T> &onlyReadArray)
{
	int nCol = getFieldCol(szField);
	FieldInfo info = getFieldInfo(nCol);
	if (info != NULL && info->getType() == FIELD_DATA)
	{
		AutoData data = getDataBuffer(nCol);
		bool bRead = true;
		if (data && data->dataSize()>0)
		{
			data->seek(0);
			bRead = data->readArray(onlyReadArray);
		}
		return bRead;
	}
	return false;
}
//-------------------------------------------------------------------------
//template<>
//Hand<BaseRecord>::operator bool() const
//{
//	return mUse && mUse->classObj && !mUse->classObj->IsNull();
//}
//typedef Hand<BaseRecord>	ARecord;
//typedef ARecord				AutoRecord;

template<>
Auto<BaseRecord>::operator bool() const
{
	return m_pBase!=NULL && !m_pBase->IsNull();
}

typedef Auto<BaseRecord>	ARecord;
typedef ARecord							AutoRecord;
//-------------------------------------------------------------------------

#endif //_INCLUDE_BASERECORD_H_