#ifndef _INCLUDE_DBOPERATE_H_
#define _INCLUDE_DBOPERATE_H_

#include "ServerEvent.h"
#include "DBCallBack.h"

using namespace Logic;
//-------------------------------------------------------------------------
enum eDBResultType
{
	eNoneError,
	eRecordCreateFail,
	eRecordNotInRange,
	eRecordRestoreDataFail,
	eRecordKeyError,
	eRecordKeyRangeError,
	eRecordAlreadyExist,
	eRecordNoExist,
	eRecordNoExistInNode,
	eRecordNotNeedUpdate,
	eRecordDataNotEligible, //10
	eTableNoExist,
	eSourceCodeError,
	eRecordDeleteFail,
	eDBNodeNoExist,
	eSourceCodeNotFinish,
	eTableIndexNull,
	eCreateTableFail,
	eNoSetNewTable,
	eNoSetRequestRecord,
	eRecordSaveToUpdateDataFail, //20
	eFieldCheckFail,
	eFieldNoExist,
	eFieldNotSet,
	eFieldTypeNotNumber,
	eDBOperateResultFail,
	eDBOperateNoSet,
	eDBOperateNoExist,
	eDBOperateParamNoSet,
	eDBOperateParamError,
	eDefaultNotSet,				//30
	eRecordGrowthCreateFail,
	eIndexNoExist,
	eRequestOverTime,
	eWaitFinish,
	eRecordCountOverLimit,
	eUnknowError,
	eDBDisconnect,	
	eDBErrorMax,
};
enum eRecordOperateMode
{
	eRecordSaveNoReplace,
	eRecordReplaceSave,
	eRecordUpdate,
	eRecordGrowthCreate,
	eRecordDelete,
	eRecordUpdateOrInsert,
};
//-------------------------------------------------------------------------
enum eModifyDataMode
{
	eAlwaySetMode,						// 直接设置为指定数值
	eIncreaseOrDecreaseMode,			// 在原数值上增减
	eIncreaseOrDecreaseByLimitMode,		// 增减时，如果超出临界值不修改
	eIncreaseOrDecreaseToLimitMode,		// 增减时，如果超出临界值，修改为临界值
};
//-------------------------------------------------------------------------
class DBOperate : public tServerEvent
{
public:
	DBOperate()
		: mResultType(eNoneError)
	{

	}

public:
	virtual bool _Serialize(DataStream *destData)
	{
		if (!Logic::tServerEvent::_Serialize(destData))
			return false;

		destData->writeString(mTableIndex);
		destData->writeString(mRecordKey);
		return true;
	}
	virtual bool _Restore(DataStream *scrData)
	{
		AssertEx(0, "No use");
		return false;
	}

protected:
	virtual void InitData()override
	{
		tServerEvent::InitData();
		mTableIndex.setNull();
		mRecordKey.setNull();
		mErrorInfo.setNull();
		mCallBack.cleanup();
		mResultType = eNoneError;
		mResultTable.setNull();
		mResultRecord.setNull();
		mResultNiceData.setNull();
	}

	virtual void Dump() override
	{
		tServerEvent::Dump();
		TABLE_LOG("Table [%s], Record key [%s]", mTableIndex.c_str(), mRecordKey.c_str());
	}

public:
	AString			mTableIndex;
	AString			mRecordKey;

public:
	AString			mErrorInfo;
	DBCallBack		mCallBack;

public:
	int				mResultType;
	ABaseTable		mResultTable;
	ARecord			mResultRecord;
	AutoNice		mResultNiceData;
};

typedef Hand<DBOperate> HandDBOperate;


#endif //_INCLUDE_DBOPERATE_H_