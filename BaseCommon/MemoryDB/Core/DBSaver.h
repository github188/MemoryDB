/********************************************************************
	created:	2014/07/26
	created:	26:7:2014   22:37
	filename: 	H:\RemoteGame\BaseCommon\ServerBase\DBWork\DBSaver.h
	file path:	H:\RemoteGame\BaseCommon\ServerBase\DBWork
	file base:	DBSaver
	file ext:	h
	author:		Yang Wenge
	
	purpose:	数据保存对象, DB 数据落地功能, 用于落地线程
	NOTE:		可以同步阻塞式保存, MemoryDB 内部异步调用			
				1 数据固定每个记录数据由 ID, DATA 两个数据组成
				2 ID 由两种 数字(64整数)和字符串
				3 DATA 字符串类型				
				4 Memory DB 为每个内存表实例一个保存对象

	讨论:		如果使用一个MYSQL来完成所有落地时, 
				1 存取将是非常繁忙, 
				2 设计框架比较复杂
				3 任务数据会占用比较多的内存

	结论:		落地处理, 如果全部使用本地磁盘文件, 直接处理, 比较理想
*********************************************************************/
#ifndef _INCLUDE_DBSAVER_H_
#define _INCLUDE_DBSAVER_H_

#include "Hand.h"
#include "LoadDataCallBack.h"
#include "AutoString.h"

class NiceData;
class DataStream;
class tBaseTable;

class tDBSaver : public Base<tDBSaver>
{
public:
	virtual bool Start(NiceData &initParam) = 0;
	virtual void Log(const char *, ...) = 0;

	// 新建DB表
	virtual bool InitReadyNewDB(const char *szDBName, tBaseTable *memoryTable, AString tableData) = 0;

	virtual const char* GetDBName() = 0;

	virtual void Process() {}

public:
	virtual bool SaveData(Int64 keyID, void *pData, DSIZE size) = 0;
	virtual bool SaveData(const char *szKeyID, void *pData, DSIZE size) = 0;

	virtual bool LoadData(Int64 keyID, DataStream *destData) = 0;
	virtual bool LoadData(const char *szKeyID, DataStream *destData) = 0;

	virtual bool DelectData(const char *szKeyID, UInt64 extValue) = 0;

	// 用于MemoryDB 初始加载所有数据记录
	virtual bool LoadAllData(LoadDataCallBack &loadCallBack, AString &errorInfo) = 0;
};


#endif //_INCLUDE_DBSAVER_H_