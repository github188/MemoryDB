//#include "IDNiceData.h"
//
//AData& IDNiceData::getOrCreate(const char* key)
//{
//	int nKey = MAKE_INDEX_ID(key);
//	AData *dest = mDataMap.findPtr(nKey);
//
//	if (NULL==dest)
//	{
//		mDataMap.insert(nKey, AData());
//		return mDataMap.find(nKey);
//	}
//	else
//	{
//		return *dest;
//	}
//}
//
////-------------------------------------------------------------------------*/
//#define AUTODATA_COUNT_TYPE		unsigned char
//#define AUTODATA_COUNT_MAX		(0xff)
//
//bool IDNiceData::serialize(DataStream *destData, bool bKeyIndex) const
//{
//	//if (!bKeyIndex)
//	//{
//	//	ERROR_LOG("IDNiceData 不支持 使用名称序列");
//	//	return false;
//	//}
//	
//	// 兼容NiceData中的序列	
//	//destData->write((byte)ID_NICEDATA);
//
//	AUTODATA_COUNT_TYPE s = (AUTODATA_COUNT_TYPE)count();
//	AssertEx(s<=AUTODATA_COUNT_MAX, "data count too most");
//	size_t sizePos = destData->tell();
//	destData->write(s);
//
//	AUTODATA_COUNT_TYPE saveCount = 0;
//
//	for (auto it=mDataMap.begin(); it.have(); it.next())
//	{
//		const auto &val = it.Value();
//		// 如果字段不存在,或不需要序列化, 一般用于保存特殊数据, 如脚本智能对象
//		FieldInfo f = val.mVal._getField();
//		if (!f || !f->needSaveToBuffer())
//		{			
//			continue;
//		}
//
//		++saveCount;
//		
//		destData->write((int)val.mKey);
//
//		if (!val.mVal.serialize(destData))
//			return false;
//	}
//
//	if (saveCount<s)
//	{
//		size_t nowPos = destData->tell();
//		destData->seek(sizePos);
//		destData->write(saveCount);
//		destData->seek(nowPos);
//	}
//
//	return true;
//}
//
//bool IDNiceData::restore(DataStream *scrData)
//{
//	// 可以读取恢复 NiceData的数据
//	//byte type = 0;
//	//if (!scrData->read(type))
//	//	return false;	
//
//	//if (type!=ID_NICEDATA && type!=NICEDATA)
//	//{
//	//	ERROR_LOG("Type [%d]: IDNiceData 无法恢复, 错误的数据", type);
//	//	return false;
//	//}
//
//	//if (type==NICEDATA)
//	//{
//	//	WARN_LOG("Type [%d]: IDNiceData 读取恢复 NiceData 的数据");
//	//}	
//
//	mDataMap.clear(false);
//
//	AUTODATA_COUNT_TYPE s = 0;
//
//	if (!scrData->read(s))
//		return false;
//
//	for (size_t i=0; i<s; ++i)
//	{
//		int k = 0;
//
//		//if (type == ID_NICEDATA)
//		//{
//			if (!scrData->read(k) )
//				return false;
//		//}
//		//else
//		//{
//		//	AString keyName;
//		//	if (!scrData->readString(keyName))
//		//		return false;
//
//		//	k = MAKE_INDEX_ID(keyName.c_str());
//		//}
//
//		mDataMap.insert(k, AData());
//		if (!mDataMap.find(k).restore(scrData))
//			return false;
//	}
//
//	return true;
//}
