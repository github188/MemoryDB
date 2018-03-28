#ifndef _INCLUDE_TABLEFIELDINFO_H_
#define _INCLUDE_TABLEFIELDINFO_H_

#include "FieldInfo.h"
#include "AutoObjectFieldInfo.h"
//#include "NiceTable.h"

#include "BaseTable.h"
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//class BaseCommon_Export TableFieldInfo : public AutoObjFieldInfo<AutoTable, FIELD_TABLE>
//{
//	typedef AutoObjFieldInfo<AutoTable, FIELD_TABLE> base;
//
//public:
//	virtual const char* getTypeString()const{ return TYPE_TABLE_NAME; }
//
//public:
//	virtual bool saveData(void *data, DataStream *destData) const;
//
//	virtual bool restoreData(void *data, DataStream *scrData);
//
//	virtual bool set( void *data, const char* szVal );
//	virtual bool set(void *data, const AString &strVal);
//
//	virtual bool get(void *data, AString &resultString) const;
//
//	virtual AString getTypeParam() const;
//	virtual void setTypeParam(const AString &typeParam);
//
//	virtual void CopyData(const tFieldInfo *pScrFieldInfo);
//
//public:
//	AutoField	mSubTableField;
//};
////----------------------------------------------------------------------------------------------------------------------
//
//class TableFieldInfoFactory : public tFieldInfoFactory
//{
//public:
//	TableFieldInfoFactory(){ }
//
//	virtual int getType() const { return FIELD_TABLE; }
//	virtual const char* getTypeName() const { return TYPE_TABLE_NAME; }
//	virtual tFieldInfo*	createFieldInfo()
//	{
//		return MEM_NEW TableFieldInfo();
//	}
//};
enum 
{
	_SubTableNull = 0, 
	_SubTableDefault = 1,
};
//----------------------------------------------------------------------------------------------------------------------
// 用于内存 DB
// 字段信息只保存校验码
// NOTE: 此表格字段, 序列变化增量数据, 如果全部数据序列, 则直接全部置为变化状态
//		 如果一条记录未在表格中, 则默认创建记录处理
//		 删除子表记录操作通过其他命令删除
//-------------------------------------------------------------------------
class BaseCommon_Export DBTableFieldInfo : public AutoObjFieldInfo<ABaseTable, FIELD_TABLE>
{
	typedef AutoObjFieldInfo<ABaseTable, FIELD_TABLE> base;

public:
	virtual const char* getTypeString()const{ return TYPE_TABLE_NAME; }

	virtual int getMaxLength() const override { return DB_BLOB_FIELD_DEFAULT_LENGTH; }
	virtual int getDBDataLength() const override { return getMaxLength()+sizeof(DSIZE); }	// 加4个字节保存长度

	virtual bool saveField( DataStream *destData ) const;
	virtual bool restoreFromData( DataStream *scrData );

	virtual AutoTable CreateFieldTable();

public:
	virtual bool saveData(void *data, DataStream *destData) const;

	virtual bool restoreData(void *data, DataStream *scrData);

	virtual bool set( void *data, const char* szVal );
	virtual bool get( void *data, AString &resultString ) const;

	AString getTypeParam() const;

	void setTypeParam( const AString &typeParam );

	void CopyData( const tFieldInfo *pScrFieldInfo );

    virtual bool SaveDataToDB(tBaseTable *pScrTable, DataStream *pDBData) override
    {
        int checkCode = pScrTable->GetField()->GetCheckCode();
        pDBData->write(checkCode);
        if (!pScrTable->SaveData(pDBData))
        {
            ERROR_LOG("记录中的表格保存到数据失败");
            return false;
        }
        return true;
    }

    virtual bool RestoreFromDB(DataStream *pScrDBData, tBaseTable *destTable) override
    {
        pScrDBData->seek(0);

        int checkCode = 0;
        READ(pScrDBData, checkCode);

        if (checkCode!=destTable->GetField()->GetCheckCode())
        {
            ERROR_LOG("记录子表的字段检验失败> 字段[%s] record code %d != %d", getName().c_str(), 
                checkCode, destTable->GetField()->GetCheckCode() );
            return false;
        }
        if (!destTable->LoadFromData(pScrDBData))
        {
            ERROR_LOG("记录子表恢复还原> 字段[%s] table [UNKNOW]", getName().c_str());
            destTable->ClearAll();
            return false;
        }
        return true;
    }

    virtual bool CheckField() 
    {
        if (mSubTableField )            
        {
            if (!mSubTableField->check())
            {
                ERROR_LOG("子表格字段检查失败 [%s]", getName().c_str());
                return false;
            }
            return true;
        }
        else
            ERROR_LOG("子表格字段%s为空", getName().c_str());
        return false;
    }

public:
	AutoField	mSubTableField;
};
//----------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------
class DB_TableFieldInfoFactory : public tFieldInfoFactory
{
public:
	DB_TableFieldInfoFactory(){ }

	virtual int getType() const { return FIELD_TABLE; }
	virtual const char* getTypeName() const { return TYPE_TABLE_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW DBTableFieldInfo();
	}
};
//-------------------------------------------------------------------------
#endif