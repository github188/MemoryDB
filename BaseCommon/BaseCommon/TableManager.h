/********************************************************************
	created:	2010/04/29
	created:	29:4:2010   10:37
	filename: 	d:\Work\common\DataBase\TableManager.h
	file path:	d:\Work\common\DataBase
	file base:	TableManager
	file ext:	h
	author:		���ĸ�
	
	purpose:	���ȫ�ֹ������,���Ա�����ֱ�񼰴���,�ҿ��ṩ���ö�ȡ
*********************************************************************/
#ifndef _TABLEMANAGER_H_
#define _TABLEMANAGER_H_

#include "BaseCommon.h"
#include "baseTable.h"
#include "EasyMap.h"
#include "FactoryTemplate.h"

//#include <hash_map>

//-------------------------------------------------------------------------
#if __WINDOWS__ || __SERVER__
#	define DEBUG_SAVE_CONFIG_TABLE		0
#	define SAVE_DEBUG_PATH				"d:/"
#else
#	define DEBUG_SAVE_CONFIG_TABLE		0
#endif

#define DEBUG_FIELD_INFO			0
//-------------------------------------------------------------------------
typedef EasyHash<AString, AutoTable> TableMap;

//-------------------------------------------------------------------------
class BaseCommon_Export DataTableManager
{
public:
	DataTableManager();
	virtual ~DataTableManager();;

public:
	void Release(void) { delete this; }

	void Upate(void* param);

	virtual void Log(const char *szInfo, ...);

	virtual AutoData ReadFile(const char *szPathFileName );

public:

	bool LoadTable( const char* configTableFile,const char* type = "CSV" );
	// ��һ�����ñ���,����������Դ��
	void LoadTable( AutoTable configTable );

	void ReadyTable(bool bChangeIDIndex);

	// �õ����ñ�
	AutoTable GetTable( const char* tableIndex, const char* errorInfo = NULL );

	AutoRecord GetRecord(const char* tableIndex, const char* recordIndex);
	AutoRecord GetRecord(const char* tableIndex, int nRecordIndex);

	Data GetData(const char* tableIndex, const char* recordIndex, const char* fieldName);
	Data GetData(const char* tableIndex, int recordIndex, const char* fieldName);

	Data GetData(const char* tableIndex, const char* recordIndex, const AString &fieldName)
	{
		return GetData(tableIndex, recordIndex, fieldName.c_str());
	}
	Data GetData(const char* tableIndex, int recordIndex, const AString &fieldName)
	{
		return GetData(tableIndex, recordIndex, fieldName.c_str());
	}

	Data GetData( const char* tableIndex, const char* recordIndex, int field );
	Data GetData( const char* tableIndex, int recordIndex, int field );

	// �޸�����, �����޸����ñ�� 
	//INT	INT	STRING	STRING	STRING	STRING	FLOAT	STRING
	//INDEX	MODIFYMODE	TABLEINDEX	CONFIGINDEX	FIELDNAME	MODIFYVALUE	AFFECTPARAM	INFO
	bool ModifyTableData(AutoTable modifySetTable);

	AutoTable GenerateInitNewModifyTable();

public:
	// ����һ���µı��,�����ǰ������ɾ����ǰ��
	virtual AutoTable CreateNewTable( const char* tableIndex,const char* tableType = "Nice" );

	virtual AutoTable CreateNewObject(const char *szType)
	{
		return tBaseTable::NewBaseTable();
	}

	// �ͷ�һ����
	bool ReleaseTable( const char* tableIndex,bool nowFree = false );

	//�Ƿ����ĳ�����͵ı�
	bool IsExists( const char* tableIndex ){ return mTableMap.find(tableIndex); }

	// ������м��ص����ݱ��
	void ReleaseAllTable(void){ mTableMap.clear(); }

	bool InsertTable( const char* tableIndex,AutoTable hTable,bool bReplace );

	TableMap& GetTableMap(void){ return mTableMap; }

protected:
	TableMap            mTableMap;
};

#ifndef __LINUX__
template<>
void AutoPtr<DataTableManager>::FreeClass()
{
	getPtr()->Release();
}
template<>
void AutoPtr<DataTableManager>::FreeUse()
{
	mUse->Release();
}

#endif
//------------------------------------------------------------------------------------
class BaseCommon_Export ConfigManager : public DataTableManager
{
public:
	virtual AutoTable CreateNewObject( const char* tableIndex ) override;
};
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
#if FIELD_INFO_THREAD_SAFE
struct BaseCommon_Export FieldThreadTool
{
	FieldInfoManager		mFieldMgr;
	//NiceDataField				mNiceFieldMgr;
#if DEBUG_FIELD_INFO
	size_t							mThreadID;
#endif

	FieldThreadTool()
#if DEBUG_FIELD_INFO
		: mThreadID(0)
#endif
	{}

	~FieldThreadTool()
	{}

	void ReadySet();

	static void Init();
	static void Free();
	static FieldInfoManager* ThreadFieldMgr();

	static size_t msToolTls;
};
#else
struct FieldThreadTool
{
	void ReadySet(){}
	static void Init(){}
	static void Free(){}
	static FieldInfoManager* ThreadFieldMgr(){}
};
#endif
//------------------------------------------------------------------------------------
class BaseCommon_Export TableManager : public ConfigManager
{
	friend struct FieldThreadTool;
protected:
	static TableManager      *ms_Singleton;

public:
	TableManager();
	~TableManager();

	static TableManager* getSingletonPtr(void)
	{
		AssertEx( ms_Singleton , "no exist singleton object");
		return ms_Singleton;
	}
	static TableManager& getSingleton(void)
	{  
#pragma warning(push)
#pragma warning(disable:6011)
		//AssertEx( ms_Singleton , "no exist singleton object");
		if (ms_Singleton==NULL)
			new TableManager();

		return ( *ms_Singleton );

#pragma warning(pop)
	}  

public:
	static void	SetLog( SaveTableLog *tableLog );
	static void SetDefaultPath( const char* path );
	static const EasyString& GetDefaultPath(void);

	static void FreeAll();

protected:
	EasyString		mDefaultPath;

	FieldThreadTool		mFieldTool;
};
//------------------------------------------------------------------------------------

#endif	//_TABLEMANAGER_H_
