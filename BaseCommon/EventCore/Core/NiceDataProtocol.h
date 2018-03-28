#ifndef _INCLUDE_NICEDATAPROTOCOL_H_
#define _INCLUDE_NICEDATAPROTOCOL_H_

#include "NiceData.h"
#include "DataStream.h"
//#include <hash_map>
//-------------------------------------------------------------------------*/
// 用于网络协议数据序列化优化
// 目前主要用于省略数据名称的发送
// 理论可以优化数据类型，每个数据可节省一个字节，目前设计上简化及数据安全考虑 目前不优化
//-------------------------------------------------------------------------*/
////class NiceDataProtocol : public Base<NiceDataProtocol>
////{
////public:
////	struct DataInfo 
////	{
////		//AutoFieldInfo	mType;
////		AString			mName;
////		int				mID;
////
////		DataInfo()
////			: mID(-1)
////		{
////
////		}
////	};
////
////public:
////	// 返回增加个数
////	int ModifyDataInfo(NiceData &scrData)
////	{
////		int count = scrData.count();
////		int addCount = 0;
////		for (int i=0; i<count; ++i)
////		{
////			auto it = mDataInfoList.find(scrData.getKey(i));
////			if (it!=mDataInfoList.end())
////			{
////				continue;
////			}
////			DataInfo info;
////
////			AString name = scrData.getKey(i);
////
////			info.mName = name;
////			info.mID = mInfoArray.size();
////			mInfoArray.push_back(info);
////			mDataInfoList[name] = info;										
////			
////			++addCount;
////		}
////		return addCount;
////	}
////
////	bool Serialize(NiceData &scrData, DataStream *destData)
////	{
////		int count = scrData.count();
////		if (count>0xff)
////			return false;
////		destData->write((byte)count);
////		for (int i=0; i<count; ++i)
////		{			
////			AData &d = scrData.get(i);			
////			if (!d.empty())
////			{
////				AString name = scrData.getKey(i);
////				
////				auto it = mDataInfoList.find(name);
////				if (it!=mDataInfoList.end())
////				{
////					DataInfo &proInfo = it->second;
////					destData->write((byte)proInfo.mID);
////
////					if (!d.serialize(destData))
////						return false;
////				}
////				else
////					return false;
////			}
////			else
////				destData->write((byte)0);
////		}
////
////		return true;
////	}
////
////	bool Restore(DataStream *scrData, NiceData &destNice)
////	{
////		byte count = 0;
////		if (!scrData->read(count))
////			return false;
////
////		for (int i=0; i<count; ++i)
////		{
////			byte dataID = -1;
////			if (!scrData->read(dataID))
////				return false;
////
////			if (dataID!=0xff)
////			{
////				if (dataID>=0 && mInfoArray.size())
////				{
////					DataInfo &info = mInfoArray[dataID];
////					AData &d = destNice.getOrCreate(info.mName);
////					if (d.restore(scrData))
////						continue;
////				}				
////			}
////
////			return false;
////		}
////		return true;
////	}
////
////	bool _SaveProtocol(DataStream *destData)
////	{
////		byte count = mDataInfoList.size();
////		destData->write(count);
////		for (auto it=mDataInfoList.begin(); it!=mDataInfoList.end(); ++it)
////		{
////			DataInfo &info = it->second;
////			destData->write((byte)info.mID);
////			destData->writeString(info.mName);
////		}
////		return true;
////	}
////
////	bool _RestoreProtocol(DataStream *scrData)
////	{
////		mDataInfoList.clear();
////		mInfoArray.clear();
////
////		byte count = 0;
////		if (!scrData->read(count))
////			return false;
////
////		for (int i=0; i<count; ++i)
////		{
////			DataInfo info;
////			byte t, id;
////			READ(scrData, id); 
////			READ(scrData, t);
////			if (!scrData->readString(info.mName))
////				return false;
////
////			info.mID = id;
////
////			mDataInfoList[info.mName] = info;
////			if (mInfoArray.size()<=id)
////				mInfoArray.resize(id+1);
////
////			mInfoArray[id] = info;
////		}
////		return true;
////	}
////
////public:
////	typedef stdext::hash_map<AString, DataInfo>	InfoMap;
////	typedef Array<DataInfo>	 InfoArray;
////	InfoMap			mDataInfoList;
////	InfoArray		mInfoArray;
////	AString			mEventName;
////};
////
////typedef Hand<NiceDataProtocol>		ANiceDataProtocol;

//class NiceProtocolMgr : public Base<NiceProtocolMgr>
//{
//public:
//	 // 如果不存在，则返回新整理的协议
//	 ANiceDataProtocol SerializeEventAndModifyProtocol(AutoEvent &scrEvent, DataStream *destData, bool bModifyProtocol)
//	 {
//		 ANiceDataProtocol p = GetProtocol(scrEvent);
//		 bool bRe = false;
//		 if (p)
//			 bRe = SerializeEvent(p, scrEvent, destData);
//
//		 if (!bRe && bModifyProtocol)
//		 {
//			 p = MEM_NEW NiceDataProtocol();
//			 if (p->ModifyDataInfo(scrEvent->GetData()))
//			 {
//				 mProtocolList[scrEvent->GetNameIndex()] = p;
//			 }
//			 else
//			 {
//				 ERROR_LOG("整理数据序列协议失败>[%s]", scrEvent->GetEventNameInfo());
//				 return ANiceDataProtocol();
//			 }
//			 if (!SerializeEvent(p, scrEvent, destData))
//			 {
//				 ERROR_LOG("根据协议序列失败>[%s]", scrEvent->GetEventNameInfo());
//				 return ANiceDataProtocol();
//			 }
//		 }
//
//		 return p;
//	 }
//
//	 bool RestoreNiceData(DataStream *scrData, AutoEvent &destEvent)
//	 {
//		  ANiceDataProtocol p = GetProtocol(scrEvent);
//		  if (p)
//			  return p->Restore(scrData, destEvent->GetData());
//		  return false;
//	 }
//
//	 ANiceDataProtocol GetProtocol(AutoEvent &scrEvent)
//	 {
//		 auto it = mProtocolList->find(scrEvent->GetNameIndex());
//		 if (it!=mProtocolList.end())
//			 return it->second;
//		 return ANiceDataProtocol();
//	 }
//
//	 bool SerializeThis(DataStream *destData)
//	 {
//
//	 }
//
//	 bool RestoreThis(DataStream *scrData)
//	 {
//
//	 }
//
//protected:
//	 bool SerializeEvent(ANiceDataProtocol &protocol, AutoEvent  &scrEvent, DataStream *destData)
//	 {
//		 return protocol->Serialize(scrEvent->GetData(), destData);
//	 }
//
//public:
//	typedef stdext::hash_map<int, ANiceDataProtocol>	mProtocolList;
//};

#endif //_INCLUDE_NICEDATAPROTOCOL_H_