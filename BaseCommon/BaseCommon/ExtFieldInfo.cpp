
#include "DataBuffer.h"
#include "Data.h"
//#include "DBDataHelper.h"
#include "ExtFieldInfo.h"
#include "TableTool.h"


//bool BinaryDataFieldInfo::set(void *data, const void* obj, const type_info &info) 
//{
//	if (typeid(DataStream)==info)
//	{
//		DataStream *scrData = (DataStream*)obj;		
//		return	_saveData(data, scrData->data(), scrData->size());
//	}
//	else if (typeid(AutoData)==info)
//	{
//		AutoData &scrData = *((AutoData*)obj);
//		if (scrData)
//			return _saveData(data, scrData->data(), scrData->size());
//	}
//	//else if (typeid(DBDataHelper)==info)
//	//{
//	//	DBDataHelper *p = (DBDataHelper*)obj;
//	//	if (p->data()==_ptr(data)) // use set col update state.
//	//		return true;
//	//	return _saveData(data, p->data(), p->size());
//	//}
//
//	return false;
//}
//
//bool BinaryDataFieldInfo::get(void *data, void* obj, const type_info &info) const 
//{
//	if (obj==NULL)
//		return false;
//
//	if (typeid(DataStream)==info)
//	{
//		DataStream *scrData = (DataStream*)obj;		
//		scrData->resize(getLength());
//		scrData->seek(0);
//		scrData->_write((void*)_ptr_const(data), getLength());
//		return	true;
//	}
//	else if (typeid(AutoData)==info)
//	{
//		AutoData &scrData = *((AutoData*)obj);
//		if (!scrData)
//			scrData = MEM_NEW DataBuffer(getLength());
//
//		scrData->_write((void*)_ptr_const(data), getLength());
//
//		return true;		
//	}
//	//else if (typeid(DBDataHelper)==info)
//	//{
//	//	// 这是当前最佳的取值方法 ~V~, 直接使用记录获取　DBDataHelper
//	//	DBDataHelper *p = (DBDataHelper*)obj;
//	//	p->InitData((char*)_ptr_const(data), getLength());
//	//	return true;
//	//}
//
//
//	return false;
//}
//
//bool BinaryDataFieldInfo::get( void *data, AString &strAString ) const
//{
//	strAString = TableTool::BinaryToString(_ptr_const(data), getLength(), "0x");
//	return true;
//}
//
//bool BinaryDataFieldInfo::setData( void *data, const Data *dataVal )
//{
//	if (NULL!=dataVal->_getFieldInfo() 
//		&& dataVal->_getFieldInfo()->getType()==getType()
//		&& dataVal->_getFieldInfo()->getLength()<=getLength()
//		)
//	{
//		memcpy(_ptr(data), dataVal->_getData(), dataVal->_getFieldInfo()->getLength() );
//		return true;
//	}
//
//	return false;
//}

int CharStringFieldInfo::getIndexValue( void* data ) const
{
	return MAKE_INDEX_ID(_ptr_const(data));
}

int CharStringFieldInfo::getIndexValue( int nIndex ) const
{
	StringTool temp; return MAKE_INDEX_ID( temp.ToString(nIndex) );
}

int CharStringFieldInfo::getIndexValue( const char* strIndex ) const
{
	return MAKE_INDEX_ID(strIndex);
}
