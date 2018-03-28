
#ifndef _RECORD_FIELD_INFO_H_
#define _RECORD_FIELD_INFO_H_
#include "FieldInfo.h"
#include "AutoObjectFieldInfo.h"
//#include "NiceTable.h"
#include "BaseTable.h"
#include "BaseRecord.h"
class BaseCommon_Export AutoRecordFieldInfo : public AutoObjFieldInfo<AutoRecord, FIELD_RECORD>
{
	typedef AutoObjFieldInfo<AutoRecord, FIELD_RECORD> base;

public:
	AutoRecordFieldInfo();
	~AutoRecordFieldInfo();

public:
	virtual const char* getTypeString()const{ return TYPE_RECORD_NAME; }
	virtual int getMaxLength() const override { return DB_BLOB_FIELD_DEFAULT_LENGTH; }
	virtual int getDBDataLength() const override { return getMaxLength()+sizeof(DSIZE); }	// 加4个字节保存长度

public:
	virtual bool saveData(void *data, DataStream *destData) const;

	virtual bool restoreData(void *data, DataStream *scrData);

private:
	AutoTable	mOwnerTable;
};
//----------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------
class AutoRecordFieldInfoFactory : public tFieldInfoFactory
{
public:
	AutoRecordFieldInfoFactory(){ }

	virtual int getType() const { return FIELD_RECORD; }
	virtual const char* getTypeName() const { return TYPE_RECORD_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW AutoRecordFieldInfo();
	}
};
//-------------------------------------------------------------------------
#endif