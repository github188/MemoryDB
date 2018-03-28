#ifndef _INCLUDE_DBFIELDTALBE_H_
#define _INCLUDE_DBFIELDTALBE_H_

#include "TableFieldInfo.h"
#include "IndexBaseTable.h"
//----------------------------------------------------------------------
// ����DB�ӱ��ֶ�
//-------------------------------------------------------------------------
class ExDBTableFieldInfo : public DBTableFieldInfo
{
	typedef DBTableFieldInfo base;

public:
	virtual AutoTable CreateFieldTable() override;

	virtual bool set( void *data, const char* szVal ){ return false; }
	virtual bool get( void *data, AString &resultString ) const { return false;}

	void setTypeParam( const AString &typeParam )
	{
		TABLE_LOG("No use this function ExDBTableFieldInfo::setTypeParam");
	}

	void CopyData( const tFieldInfo *pScrFieldInfo )
	{
		base::CopyData(pScrFieldInfo);
		const ExDBTableFieldInfo *p = dynamic_cast<const ExDBTableFieldInfo*>(pScrFieldInfo);
		if (p!=NULL)
		{
			mDBTable = p->mDBTable;
			mDBTableName = p->mDBTableName;
		}
	}

    virtual bool SaveDataToDB(tBaseTable *pScrTable, DataStream *pDBData) override;
    virtual bool RestoreFromDB(DataStream *pScrDBData, tBaseTable *destTable) override;

    // NOTE: ���ֶμ��ʱ,����Ҫ���м��, ����DB����ֶε��� updateFieldʱ,����ռ�¼, ������¼���ݴ���
     virtual bool CheckField() override { return mDBTable; }

public:
	AutoTable	mDBTable;
	AString mDBTableName;
};
/*/----------------------------------------------------------------------
ʹ�ö���DB��񱣴��ӱ�
1 ���������򴴽�ʱ, ȫ�ӱ�IDΨһ, �������������ӱ�DB����
2 �ֶ��ӱ���, ֻ�Ǳ������ö�Ӧ ����DB���¼����
3 ���ֶα���IDֻ��Ϊ����
4 ���ֶε�ǰDB�ڵ��У���Ӧ��ǰ�ڵ��е�DB����֧�ֶ�̬������ϣ��
//----------------------------------------------------------------------*/
class ExDBTableFieldInfo;
class DBFieldTable : public NormalBaseTable
{
public:
	DBFieldTable(ExDBTableFieldInfo *pInfo)
		: NormalBaseTable(eInitNullField)
		, mpDBTableFieldInfo(pInfo)
		, mbChanged(false)
	{
		
	}

	virtual const char* GetTableType(){ return "DBFieldTable"; }
	virtual const char* GetTableName() const { return ""; }
	virtual void SetTableName(const char *szTableName){}

    virtual bool NeedCheckRecordUpdateState() const override { return false; }

public:
	virtual bool HasDeleteRecord(){ return mbChanged; }
	virtual void ClearDeleteRecordList(){ mbChanged = false; }

    virtual ARecord NewRecord() override
    {
        if (mpDBTableFieldInfo->mDBTable)
            return mpDBTableFieldInfo->mDBTable->NewRecord();
        else
            ERROR_LOG("DB field DB table is NULL");
        return ARecord();
    }

    virtual bool AppendRecord(ARecord scrRecord, bool bReplace, bool bRestore = false) override;

	virtual ARecord GrowthNewRecord( DataStream *recordData ) override
	{
		if (mpDBTableFieldInfo->mDBTable)
		{
			ARecord r = mpDBTableFieldInfo->mDBTable->GrowthNewRecord(recordData);
			if (r)
			{
				NormalBaseTable::AppendRecord(r, true);
				mbChanged = true;
			}
			return r;
		}
		ERROR_LOG("DB field DB table is NULL");
		return ARecord();
	}
	virtual ARecord CreateRecord(Int64 nIndex, bool bReplace) override
	{
		if (mpDBTableFieldInfo->mDBTable)
		{
			ARecord r = mpDBTableFieldInfo->mDBTable->CreateRecord(nIndex, bReplace);
			if (r)
			{
				NormalBaseTable::AppendRecord(r, true);
				mbChanged = true;
			}
			return r;
		}
		ERROR_LOG("DB field DB table is NULL");
		return ARecord();
	}
	virtual ARecord CreateRecord(float fIndex, bool bReplace) override
	{
		if (mpDBTableFieldInfo->mDBTable)
		{
			ARecord r = mpDBTableFieldInfo->mDBTable->CreateRecord(fIndex, bReplace);
			if (r)
			{
				NormalBaseTable::AppendRecord(r, true);
				mbChanged = true;
			}
			return r;
		}
		ERROR_LOG("DB field DB table is NULL");
		return ARecord();
	}
	virtual ARecord CreateRecord(const char* szIndex, bool bReplace) override
	{
		if (mpDBTableFieldInfo->mDBTable)
		{
			ARecord r = mpDBTableFieldInfo->mDBTable->CreateRecord(szIndex, bReplace);
			if (r)
			{
				NormalBaseTable::AppendRecord(r, true);
				mbChanged = true;
			}
			return r;
		}
		ERROR_LOG("DB field DB table is NULL");
		return ARecord();
	}

	virtual bool DeleteRecord(ARecord record) override
	{
		if (mpDBTableFieldInfo->mDBTable)
		{
			if (mpDBTableFieldInfo->mDBTable->DeleteRecord(record))
			{
				NormalBaseTable::DeleteRecord(record);
				mbChanged = true;			
				return true;
			}
			return false;
		}
		ERROR_LOG("DB field DB table is NULL");
		return false;
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
                ERROR_LOG("�ָ�ɾ����¼KEYʧ��");
                return false;
            }
            ARecord r = GetRecord(key.c_str());
            if (r && mpDBTableFieldInfo->mDBTable)
            {
                mpDBTableFieldInfo->mDBTable->DeleteRecord(r);
                if (DeleteRecord(r))
                    mbChanged = true;
            }
        }
        return true;
    }

public:
	ExDBTableFieldInfo *mpDBTableFieldInfo;
	bool mbChanged;
};


//----------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------
class ExDBTableFieldInfoFactory : public tFieldInfoFactory
{
public:
    ExDBTableFieldInfoFactory(){ }

    virtual int getType() const { return FIELD_DB_TABLE; }
    virtual const char* getTypeName() const { return "DBTALBE"; }
    virtual tFieldInfo*	createFieldInfo()
    {
        return MEM_NEW ExDBTableFieldInfo();
    }
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_DBFIELDTALBE_H_