
#include "Data.h"

#include "FieldInfo.h"
#include "NiceData.h"
#include "BaseTable.h"

Data::operator ArrayNiceData*() const
{
    AutoArrayData d;
    get(&d, typeid(d));
    if (d)
        return d.getPtr();

    AutoNice n;
    get(&n, typeid(AutoNice));
    return dynamic_cast<ArrayNiceData*>(n.getPtr());
}

bool Data::get(int &nVal) const
{
	if (!empty())
		return _getFieldInfo()->get(_getData(), nVal);
	return false;
}

bool Data::get( float &fVal ) const
{
	if (!empty())
		return _getFieldInfo()->get(_getData(), fVal);
	return false;
}

bool Data::get(UInt64 &uResult) const 
{ 
	if (_getFieldInfo() && _getData()) return _getFieldInfo()->get(_getData(), uResult); return false; 
}

bool Data::get( bool &bVal ) const
{
	if (!empty())
		return _getFieldInfo()->get(_getData(), bVal);
	return false;
}

bool Data::get( void *obj, const type_info &typeInfo ) const
{
	if (!empty())
		return _getFieldInfo()->get(_getData(), obj, typeInfo);
	return false;
}

bool Data::get( unsigned char &bVal ) const
{
	if (!empty())
	{
		int nV = 0;
		bool b = get(nV);
		bVal = nV;
		return b;
	}
	return false;
}

Data::operator AString() const
{
	AString str;
	if (!empty())
		_getFieldInfo()->get(_getData(), str);
	return str;
}

bool Data::comType( tFieldInfo *info ) const
{
	return info->getType()==_getFieldInfo()->getType();
}

bool Data::isInt( void ) const
{
	if (_getFieldInfo()) return _getFieldInfo()->getType()==FIELD_INT; return false;
}

bool Data::isString( void ) const
{
	if (_getFieldInfo()) return _getFieldInfo()->getType()==FIELD_STRING; return false;
}

bool Data::isFloat( void ) const
{
	if (_getFieldInfo()) return _getFieldInfo()->getType()==FIELD_FLOAT; return false;
}

bool Data::isBool( void ) const
{
	if (_getFieldInfo()) return _getFieldInfo()->getType()==FIELD_BOOL; return false;
}

AString Data::string() const
{	
	 if (empty())
	 {
		 return AString();
	 }

	 //const char *szStr = _getFieldInfo()->getString(_getData());

	 //if (szStr!=NULL)
	//	 return szStr;
	 AString result;
	 if (_getFieldInfo()!=NULL && _getData()!=NULL)
		_getFieldInfo()->get(_getData(), result);
	 else
		 ERROR_LOG("字段信息为空或数据空间为空");

	 return result;

	//if (empty() || !mString.empty())
	//	return mString.c_str();
	//StringData temp;
	//_getFieldInfo()->get(_getData(), temp);
	//if (temp.mCharString)
	//	return temp.mCharString;
	//*((AString*)(&mString)) = temp.mString;
	//
	//return mString.c_str();
}

bool Data::set( void *pValue, const type_info &valueType )
{
	if (_isAData())
	{
		// mFieldCol<0 _getData() 保存的是 AData的指针
		AData *p = (AData*)mData;
		return p->set(pValue, valueType);		
	}
	else if (!empty())
	{
		bool bResult = _getRecord()->set(mFieldCol, pValue, valueType);	
		if (!bResult)		
			ERROR_LOG("Error: set data fail: TYPE>[%s], Name>[%s]", _getFieldInfo()->getTypeString(), _getFieldInfo()->getName().c_str());		

		return bResult;
	}
	else
		ERROR_LOG("Error: set data fail > [%s]", valueType.name());

	return false;
}

Data& Data::operator=( const char *szValue )
{
	if (_isAData())
	{
		AData *p = (AData*)mData;
		p->set(szValue);		

	}
	else if (!empty())
	{
		bool bResult = _getRecord()->set(mFieldCol, szValue);		
		if (!bResult)		
			ERROR_LOG("Error: set data fail: TYPE>[%s], Name>[%s]", _getFieldInfo()->getTypeString(), _getFieldInfo()->getName().c_str());		

		return *this;
	}
	else
		ERROR_LOG("Error: set data fail > [%s]", szValue);

	return *this;
}

Data& Data::operator=( const int value )
{
	if (_isAData())
	{
		AData *p = (AData*)mData;
		p->set(value);		
	}
	else if (!empty())
	{
		bool bResult = _getRecord()->set(mFieldCol, value);		
		if (!bResult)		
			ERROR_LOG("Error: set data fail: TYPE>[%s], Name>[%s]", _getFieldInfo()->getTypeString(), _getFieldInfo()->getName().c_str());		

		return *this;
	}
	else
		ERROR_LOG("Error: set data fail > [%d]", value);

	return *this;
}

Data& Data::operator=( const short value )
{
	if (_isAData())
	{
		AData *p = (AData*)mData;
		p->set(value);	

	}
	else if (!empty())
	{
		bool bResult = _getRecord()->set(mFieldCol, value);		
		if (!bResult)		
			ERROR_LOG("Error: set data fail: TYPE>[%s], Name>[%s]", _getFieldInfo()->getTypeString(), _getFieldInfo()->getName().c_str());		

		return *this;
	}
	else
		ERROR_LOG("Error: set data fail > [%d]", value);

	return *this;
}

Data& Data::operator=( const bool value )
{
	if (_isAData())
	{
		AData *p = (AData*)mData;
		p->set(value);	

	}
	else if (!empty())
	{
		bool bResult = _getRecord()->set(mFieldCol, value);		
		if (!bResult)		
			ERROR_LOG("Error: set data fail: TYPE>[%s], Name>[%s]", _getFieldInfo()->getTypeString(), _getFieldInfo()->getName().c_str());		

		return *this;
	}
	else
		ERROR_LOG("Error: set data fail > [%d]", value ? 1:0);

	return *this;
}

Data& Data::operator=( const float value )
{
	if (_isAData())
	{
		AData *p = (AData*)mData;
		p->set(value);	

	}
	else if (!empty())
	{
		bool bResult = _getRecord()->set(mFieldCol, value);		
		if (!bResult)		
			ERROR_LOG("Error: set data fail: TYPE>[%s], Name>[%s]", _getFieldInfo()->getTypeString(), _getFieldInfo()->getName().c_str());		

		return *this;
	}
	else
		ERROR_LOG("Error: set data fail > [%f]", value);

	return *this;
}

Data& Data::operator=( const UInt64 value )
{
	if (_isAData())
	{
		AData *p = (AData*)mData;
		p->set(value);	

	}
	else if (!empty())
	{
		bool bResult = _getRecord()->set(mFieldCol, (Int64)value);		
		if (!bResult)		
			ERROR_LOG("Error: set data fail: TYPE>[%s], Name>[%s]", _getFieldInfo()->getTypeString(), _getFieldInfo()->getName().c_str());		

		return *this;
	}
	else
		ERROR_LOG("Error: set data fail > [%llu]", value);

	return *this;
}

Data& Data::operator=( DataStream *value )
{
	AutoData  d(value);
	set(&d, typeid(d));
	return *this;
}

Data& Data::operator=( tNiceData *value )
{
	AutoNice  d(value);
	set(&d, typeid(d));
	return *this;
}

Data& Data::operator=( tBaseTable *value )
{
	AutoTable  d(value);
	set(&d, typeid(d));
	return *this;
}

Data& Data::operator=( const AString &szValue )
{
	if (_isAData())
	{
		AData *p = (AData*)mData;
		p->set(szValue);		

	}
	else if (!empty())
	{
		bool bResult = _getRecord()->set(mFieldCol, szValue);		
		if (!bResult)		
			ERROR_LOG("Error: set data fail: TYPE>[%s], Name>[%s]", _getFieldInfo()->getTypeString(), _getFieldInfo()->getName().c_str());		

		return *this;
	}
	else
		ERROR_LOG("Error: set data fail > [%s]", szValue.c_str());

	return *this;
}

Data::operator DataStream*() const
{
	AutoData d;
	get(&d, typeid(AutoData));
	return d.getPtr();
}

Data::operator tBaseTable*() const
{
	BaseRecord *r = _getRecord();
	if (r!=NULL)
		return r->GetFieldTable(mFieldCol);

	AutoTable t;
	get(&t, typeid(AutoTable));
	return t.getPtr();
}

Data::operator tNiceData*() const
{
	AutoNice n;
	get(&n, typeid(AutoNice));
	return n.getPtr();
}

tFieldInfo* Data::_getFieldInfo() const
{
	if (mData!=NULL)
	{		
		if (mFieldCol>=0)
		{
			BaseRecord *p = (BaseRecord*)mData;
			return p->getFieldInfo(mFieldCol);
		}
		else
		{
			AData *p = (AData*)mData;
			return p->_getField();
		}
	}
	return NULL;
}

void* Data::_getData() const
{
	if (mData!=NULL)
	{		
		if (mFieldCol>=0)
		{
			BaseRecord *p = (BaseRecord*)mData;
			return p->_getData();
		}
		else
		{
			AData *p = (AData*)mData;
			return p->_dataPtr();
		}
	}
	return NULL;
}

BaseRecord* Data::_getRecord() const
{
	if (mFieldCol>=0)
		return (BaseRecord*)mData;

	return NULL;
}

bool Data::empty() const
{
	if (mData==NULL)
		return true;
	
	if (_isAData())
	{
		AData *p = (AData*)mData;
		return p->empty();		
	}
	else if (mFieldCol>=0 )
	{
		BaseRecord *p = (BaseRecord*)mData;
		return p->getFieldInfo(mFieldCol)==NULL;
	}

	return true;
}

bool Data::setData( const Data &other )
{
	if (_isAData())
	{
		// mFieldCol<0 _getData() 保存的是 AData的指针
		AData *p = (AData*)mData;
		return p->set(other);
	}
	else if (!empty())
	{
		FieldInfo info = _getRecord()->getFieldInfo(mFieldCol);
		if (info!=NULL)
			return info->setData(_getRecord()->_getData(), &other);	
		else
			ERROR_LOG("Record no exist col %d", mFieldCol);
	}
	return false;
}
