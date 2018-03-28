
#ifndef _INCLUDE_INDEXDBRECORD_H_
#define _INCLUDE_INDEXDBRECORD_H_

#include "BaseRecord.h"
#include "BaseTable.h"
#include "BaseCommon.h"

//-------------------------------------------------------------------------
class BaseCommon_Export IndexDBRecord : public BaseRecord
{
	friend class BaseTable;
	friend class MemoryTableFieldIndex;

public:
	IndexDBRecord() : mbNeedUpdate(false), mbAppendToTable(false) {}
	IndexDBRecord( ABaseTable hOwnerTable )
		: BaseRecord()
		, mbNeedUpdate(false)
		, mbAppendToTable(false)
	{
		mOwnerTable = hOwnerTable->GetTablePtr();
	}

	virtual ~IndexDBRecord()
	{
		InitData();
		_freeData();
	}

	virtual void InitData() override
	{
		BaseRecord::InitData();
		char *pData = _getStateData();
		if (pData!=NULL)			
			memset(pData, 0, _getStateDataSize());
	}

public:
	virtual AutoField getField() const 
	{ 
		//tBaseTable *pTable = ((IndexDBRecord*)this)->GetTable();
		//if (pTable!=NULL)
		//	return pTable->GetField();
		if (mOwnerTable)
			return mOwnerTable->mField;
		return AutoField();
	}
	virtual tBaseTable* GetTable() override { return mOwnerTable->mpTable; }

	virtual tBaseTable* CreateFieldTable( int col )
	{
		mbNeedUpdate = true;
		OnNeedUpdate(true);
		return BaseRecord::CreateFieldTable(col);
	}

	tBaseTable* GetFieldTable( int col )
	{
		mbNeedUpdate = true;
		OnNeedUpdate(true);
		return BaseRecord::GetFieldTable(col);
	}

public:
	virtual bool set(int col, Int64 nVal) override;
	virtual bool set(int col, float nVal) override;
	virtual bool set(int col, const char *szVal) override;
	virtual bool set(int col, const AString &strVal) override;
	virtual bool set(int col, void *obj, const type_info &typeInfo) override;

public:
	// 序列增量变化数据 只保存或还原修改数据部分 
	virtual bool saveUpdateData( DataStream *destData );
	
#if RECORD_ONLY_CALL_RESTOREDATA
protected:
	virtual bool _updateFromData( DataStream *scrData );
public:
#else
	virtual bool updateFromData( DataStream *scrData );
#endif

	bool MakeInsertSqlString( AString &sqlString );
	bool MakeUpdateSqlString( AString &sqlString, const char* indexFieldName );	 
	bool _LinkValueAtCol( FieldInfo filedInfo, int col, AString &sqlString );	

public:
	virtual void _alloctData( int extData )
	{
		if (getField())
			BaseRecord::_alloctData(_getStateDataSize());	
	}

	virtual void _freeData() override
	{ 
		AutoField f = getField();
		if (mRecordData!=NULL && f) 
		{
			f->_freeRecordData(mRecordData);
			mRecordData = NULL;
		}
	}

	virtual char* _allotMemory(uint size) override 
	{ 
		AutoField f = getField();
		if (f) 
			return (char*)f->_allotRecordData(size); 
		return NULL;
	}

	virtual char* _getStateData(){ if (getField() && mRecordData!=NULL) return mRecordData+getField()->getDataLength(); return NULL; }
	virtual DSIZE _getStateDataSize()
	{
		if (getField())
			return (getField()->getCount()+7)/8;

		return 0;
	}
	virtual bool _setNeedUpdate(int col, bool bNeed = true)
	{
		byte *pData = (byte*)_getStateData();
		if (pData==NULL)
			return false;

		DSIZE length = _getStateDataSize();
		if (bNeed)
		{			
			int index = col;
			if ( index>=0 && index<length*8 )
			{
				pData[index/8] |= ( 1<<(index%8) );				
			}
			//mDataState.openState(col);
			OnNeedUpdate(true);
		}
		else
		{
			int index = col;
			if ( index>=0 && index<length*8 )
			{
				pData[index/8] &= ~( 1<<(index%8) );
				return true;
			}
			//mDataState.closeState(col);
		}

		return true;
	}
	virtual bool HadChanged(int col){ return _hadChanged(col); }
	virtual bool _hadChanged(int col)
	{
		AutoField f = getField();
		if (!f || f->IsSkipUpdate(col))
			return false;

		byte *pData = (byte*)_getStateData();
		if (pData==NULL)
			return false;
		
		if ( col>=0 && col<_getStateDataSize()*8 )
			return (pData[col/8] & ( 1<<(col%8) ))>0;

		return false; 
	}

	virtual void FullAllUpdate(bool bNeed);
	virtual bool NotifyChanged(int col) override { if (col>=0) return _setNeedUpdate(col); return false; }

	virtual void _Update();

	virtual void Update(){ mbNeedUpdate = true; OnNeedUpdate(true); }

	virtual bool NeedUpdate(void);

	virtual bool InTable() const override { return mbAppendToTable;  }
	virtual void SetInTable(bool bIn){ mbAppendToTable = bIn; }

	virtual void OnNeedUpdate(bool bNeedUpdate){}

	virtual void InitTablePtr(ATablePtr tablePtr){ mOwnerTable = tablePtr; }

protected:
	ATablePtr	mOwnerTable;
	bool		mbNeedUpdate;
	bool		mbAppendToTable;
};

//-------------------------------------------------------------------------

#endif //_INCLUDE_INDEXDBRECORD_H_