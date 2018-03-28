
#ifndef _INCLUDE_OPERATE_H_
#define _INCLUDE_OPERATE_H_

#include "Auto.h"
#include "MemoryDBNode.h"
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
//class BaseMemoryDB;
//namespace Logic
//{
//	class tEvent;
//}
//class tOperate : public AutoBase
//{
//public:
//	tOperate()
//		: mErrorType(eNoneError)
//	{
//
//	}
//
//	MemoryDBNode* GetDB(BaseMemoryDB *p)
//	{
//		return dynamic_cast<MemoryDBNode*>(p);
//	}
//
//public:
//	virtual bool Execute(BaseMemoryDB  *pDB, Logic::tEvent *ownerEvent){}
//	virtual bool Serialize(DataStream *destData)
//	{
//		destData->writeString(mTableIndex.c_str());
//		destData->writeString(mRecordKey.c_str());
//		destData->write((byte)mResultType);
//		return true; 
//	}
//	virtual bool Restore(DataStream *scrData) 
//	{ 
//		if (!scrData->readString(mTableIndex))
//		{
//			ERROR_LOG("ª÷∏¥±Ì∏ÒÀ˜“˝ ß∞‹");
//			return false;
//		}
//
//		if (!scrData->readString(mRecordKey))
//		{
//			ERROR_LOG("ª÷∏¥º«¬ºKEY ß∞‹");
//			return false;
//		}
//		byte t;
//		if (!scrData->read(t))
//		{
//			ERROR_LOG("ª÷∏¥ªÿ∏¥¿‡–Õ ß∞‹");
//			return false;
//		}
//
//		return true; 
//	}
//
//	virtual void OnFinish()
//	{
//		mCallBack.run(this, mErrorInfo==eNoneError);
//	}
//
//public:
//	DBCallBack		mCallBack;
//
//public:
//	AString			mTableIndex;
//	AString			mRecordKey;
//	AutoData		mBufferData;
//
//public:
//	int				mResultType;
//	AString			mErrorInfo;
//
//	ABaseTable		mResultTable;
//	ARecord			mResultRecord;
//	AutoNice		mResultData;
//
//};
//typedef Auto<tOperate>	AutoOperate;
//-------------------------------------------------------------------------

#endif //_INCLUDE_OPERATE_H_