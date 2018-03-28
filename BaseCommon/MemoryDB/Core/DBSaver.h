/********************************************************************
	created:	2014/07/26
	created:	26:7:2014   22:37
	filename: 	H:\RemoteGame\BaseCommon\ServerBase\DBWork\DBSaver.h
	file path:	H:\RemoteGame\BaseCommon\ServerBase\DBWork
	file base:	DBSaver
	file ext:	h
	author:		Yang Wenge
	
	purpose:	���ݱ������, DB ������ع���, ��������߳�
	NOTE:		����ͬ������ʽ����, MemoryDB �ڲ��첽����			
				1 ���ݹ̶�ÿ����¼������ ID, DATA �����������
				2 ID ������ ����(64����)���ַ���
				3 DATA �ַ�������				
				4 Memory DB Ϊÿ���ڴ��ʵ��һ���������

	����:		���ʹ��һ��MYSQL������������ʱ, 
				1 ��ȡ���Ƿǳ���æ, 
				2 ��ƿ�ܱȽϸ���
				3 �������ݻ�ռ�ñȽ϶���ڴ�

	����:		��ش���, ���ȫ��ʹ�ñ��ش����ļ�, ֱ�Ӵ���, �Ƚ�����
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

	// �½�DB��
	virtual bool InitReadyNewDB(const char *szDBName, tBaseTable *memoryTable, AString tableData) = 0;

	virtual const char* GetDBName() = 0;

	virtual void Process() {}

public:
	virtual bool SaveData(Int64 keyID, void *pData, DSIZE size) = 0;
	virtual bool SaveData(const char *szKeyID, void *pData, DSIZE size) = 0;

	virtual bool LoadData(Int64 keyID, DataStream *destData) = 0;
	virtual bool LoadData(const char *szKeyID, DataStream *destData) = 0;

	virtual bool DelectData(const char *szKeyID, UInt64 extValue) = 0;

	// ����MemoryDB ��ʼ�����������ݼ�¼
	virtual bool LoadAllData(LoadDataCallBack &loadCallBack, AString &errorInfo) = 0;
};


#endif //_INCLUDE_DBSAVER_H_