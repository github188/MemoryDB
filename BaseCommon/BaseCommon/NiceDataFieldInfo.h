///********************************************************************
//	created:	2011/07/22
//	created:	22:7:2011   23:21
//	filename: 	d:\Work\Common\DataBase\NiceDataFieldInfo.h
//	file path:	d:\Work\Common\DataBase
//	file base:	NiceDataFieldInfo
//	file ext:	h
//	author:		杨文鸽
//	
//	purpose:	用于NICEDATA的字段信息, 为每一种字段实例, 且数据位置为0
//*********************************************************************/
//#ifndef _INCLUDE_NICEDATAFIELDINFO_H_
//#define _INCLUDE_NICEDATAFIELDINFO_H_
//
//#include "AutoPtr.h"
//#include "FieldInfo.h"
//#include "Array.h"
//#include "EasyMap.h"
//#include "FieldTypeDefine.h"
//#include "TableTool.h"
//#include "EasyHash.h"
//#include "Lock.h"
//
////------------------------------------------------------------------------------------
//
//class BaseCommon_Export NiceDataField
//{
//public:
//	static NiceDataField& getMe(void);
//	static void FreeAllFieldInfo();
//
//public:
//	Array<AutoFieldInfo>				mFieldInfoArray;
//	EasyHash<int, AutoFieldInfo>		mFieldInfoOnTypeInfoMap;
//	CsLock								mLock;
//
//private:
//	void InitFieldInfo();
//
//public:
//	NiceDataField();
//
//	template<typename T>
//	void appendDataFieldOnTypeInfo(AutoFieldInfo hFieldInfo)
//	{
//		int key = MAKE_INDEX_ID(typeid(T).name());
//		mFieldInfoOnTypeInfoMap.erase(key);
//		mFieldInfoOnTypeInfoMap.insert(key, hFieldInfo);
//		//appendDataField(hFieldInfo);
//		hFieldInfo->setPosition(0);
//	}
//
//	template<typename T>
//	void appendDataFieldOnTypeInfo(FIELD_TYPE fieldType)
//	{
//		AutoFieldInfo info = appendDataField(fieldType);
//		appendDataFieldOnTypeInfo<T>(info);
//	}
//
//	FieldInfo getFeildInfo(int type)
//	{
//		if (type>FIELD_NULL && type<(int)mFieldInfoArray.size())
//			return mFieldInfoArray[type].getPtr();
//		
//		return NULL;
//	}
//	void appendDataField(AutoFieldInfo hFieldInfo)
//	{
//		int type = hFieldInfo->getType();
//		if ((int)mFieldInfoArray.size()<=type)
//			mFieldInfoArray.resize(type+1);
//		mFieldInfoArray[type] = hFieldInfo;
//		hFieldInfo->setPosition(0);
//	}
//
//	FieldInfo appendDataField(FIELD_TYPE fieldType);
//
//	FieldInfo getFeildInfo(const type_info &typeInfo)
//	{
//		CsLockTool l(mLock);
//		return mFieldInfoOnTypeInfoMap.find(MAKE_INDEX_ID(typeInfo.name())).getPtr();
//	}
//	void removeFieldInfo(int type);
//};
////------------------------------------------------------------------------------------
////------------------------------------------------------------------------------------
//
//
//#endif