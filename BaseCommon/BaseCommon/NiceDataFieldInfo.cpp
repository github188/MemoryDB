//
//#include "NiceDataFieldInfo.h"
//#include "BaseFieldInfo.h"
//#include "AutoObjectFieldInfo.h"
////#include "NiceTable.h"
//#include "NiceData.h"
//#include "ExtFieldInfo.h"
//
//#include "FieldInfo.h"
//
//#include "BaseTable.h"
//
//#include <windows.h>
//#include "TableManager.h"
////namespace FieldInfoName
////{
////	extern const char tableName[];
////	extern const char recordName[];
////	extern const char dataName[];
////	extern const char niceName[];
////}
//
//NiceDataField::NiceDataField()
//{
//	mFieldInfoArray.resize(FIELD_TYPE_MAX);
//
//	InitFieldInfo();
//
//	////appendDataField(MEM_NEW IntFieldInfo);
//	////appendDataField(MEM_NEW FloatFieldInfo);
//	////appendDataField(MEM_NEW StringFieldInfo);
//	////appendDataField(MEM_NEW BoolFieldInfo);
//	////appendDataField(MEM_NEW ByteFieldInfo);
//	////appendDataField(MEM_NEW ShortFieldInfo);
//	//////appendDataField(MEM_NEW CharStringFieldInfo);
//	//////appendDataField(MEM_NEW BinaryDataFieldInfo);
//	////appendDataField(MEM_NEW UINT64FieldInfo);
//
//
//	////appendDataFieldOnTypeInfo<AutoData>(MEM_NEW AutoObjFieldInfo<AutoData, FIELD_DATA, FieldInfoName::dataName>);
//	////appendDataFieldOnTypeInfo<AutoTable>(MEM_NEW AutoObjFieldInfo<AutoTable, FIELD_TABLE, FieldInfoName::tableName>);
//	////appendDataFieldOnTypeInfo<AutoRecord>(MEM_NEW AutoObjFieldInfo<AutoRecord, FIELD_RECORD, FieldInfoName::recordName>);
//	////appendDataFieldOnTypeInfo<AutoNice>(MEM_NEW AutoObjFieldInfo<AutoNice, FIELD_NICEDATA, FieldInfoName::niceName>);
//}
//
//NiceDataField& NiceDataField::getMe( void )
//{
//	NiceDataField *pMgr = FieldThreadTool::ThreadNiceFieldMgr();
//	if (pMgr!=NULL)
//		return *pMgr;
//
//	static  	NiceDataField	smFieldInfoArray;
//	return smFieldInfoArray;
//}
//
//void NiceDataField::FreeAllFieldInfo()
//{
//	getMe().mFieldInfoArray.clear();
//	getMe().mFieldInfoOnTypeInfoMap.clear();
//}
//
//void NiceDataField::removeFieldInfo( int type )
//{
//	AutoFieldInfo hInfo = getFeildInfo(type);
//	if (hInfo)
//	{
//		mFieldInfoOnTypeInfoMap.erase(MAKE_INDEX_ID(hInfo->getTypeinfo().name()));
//		if (type>=0 && type<(int)mFieldInfoArray.size())
//			mFieldInfoArray[type] = AutoFieldInfo();
//	}
//}
//
//void NiceDataField::InitFieldInfo()
//{
//	appendDataField(FIELD_INT);
//	appendDataField(FIELD_FLOAT);
//	appendDataField(FIELD_STRING);
//	appendDataField(FIELD_BOOL);
//	appendDataField(FIELD_BYTE);
//	appendDataField(FIELD_SHORT);
//	appendDataField(FIELD_UINT64);
//	//appendDataField(FIELD_BINARY);
//	appendDataField(FIELD_DOUBLE);
//	
//	appendDataFieldOnTypeInfo<AString>(FIELD_STRING);
//
//	appendDataFieldOnTypeInfo<AutoData>(FIELD_DATA);
//	appendDataFieldOnTypeInfo<AutoTable>(FIELD_TABLE);
//	appendDataFieldOnTypeInfo<AutoRecord>(FIELD_RECORD);
//	appendDataFieldOnTypeInfo<AutoNice>(FIELD_NICEDATA);
//
//	//appendDataFieldOnTypeInfo<ABaseTable>(FIELD_DB_TABLE);
//
//	appendDataFieldOnTypeInfo<int>(FIELD_INT);
//	appendDataFieldOnTypeInfo<float>(FIELD_FLOAT);
//	appendDataFieldOnTypeInfo<bool>(FIELD_BOOL);
//	appendDataFieldOnTypeInfo<byte>(FIELD_BYTE);
//	appendDataFieldOnTypeInfo<short>(FIELD_SHORT);
//	appendDataFieldOnTypeInfo<UInt64>(FIELD_UINT64);
//	appendDataFieldOnTypeInfo<double>(FIELD_DOUBLE);
//}
//
//FieldInfo NiceDataField::appendDataField( FIELD_TYPE fieldType )
//{
//	CsLockTool l(mLock);
//	tFieldInfo *pInfo = FieldInfoManager::GetMe().createFieldInfo(fieldType);
//	AssertEx (pInfo!=NULL, "字段信息不存在");
//	AutoFieldInfo info = pInfo;
//	appendDataField(info);
//	return info.getPtr();
//}
