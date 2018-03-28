/********************************************************************
	created:	2014/10/11
	created:	11:10:2014   18:00
	filename: 	D:\Code\MemoryDBServer\Server\GameServer\Server\ClientUpdateResources.h
	file path:	D:\Code\MemoryDBServer\Server\GameServer\Server
	file base:	ClientUpdateResources
	file ext:	h
	author:		Yang Wenge
	
	purpose:	�ļ����ط���
				Ϊÿ����Ҫ���ص��ļ����� UpdateResourcesFileTool, ������MAP�б���
				1 �ͻ���������Ϣ, ָ����Դ�ļ���, �������Ƶõ� FileTool��Ϣ
				2 �������ݿ���Ϣ, Ҳ��Ҫָ����Դ�ļ���, ��ȡ�� FileTool ��, ���лظ�����
				  Ϊ��֧�ֶ�̬����, ����Ҫ�ṩ��Դ��MD5У����, ��֤��Դ���ݵ�һ����!
				3 ���ڼ���MD5��ָѹ�����ݵ�MD5 CheckMD5
*********************************************************************/
#ifndef _INCLUDE_CLIENTUPDATERESOURCES_H_
#define _INCLUDE_CLIENTUPDATERESOURCES_H_

#include "FileNetTransfer.h"

#include "EventCenter.h"

using namespace Logic;
//-------------------------------------------------------------------------
class ResourcesThread;
class DBOperate;
class FileDataServerTool;
class MySqlDBTool;
//-------------------------------------------------------------------------
class UpdateResourcesFileTool : public FileNetTransferSendControl
{
public:
	UpdateResourcesFileTool(FileDataServerTool *pTool, const char *szResourceName)
		: mResourcesName(szResourceName)
		, mAllPartCount(0)
		, mpFileDataServerTool(pTool)
	{
		mZipPackData = MEM_NEW DataBuffer();
		mSendFileData = MEM_NEW DataBuffer();
	}

	virtual void OnLoadUpdateResourcesDataSucceed();

	virtual void OnLoadUpdateResourcesDataFail()
	{
		NOTE_LOG("Error: XXX ��ȡ������Դ����ʧ��");
	}

	virtual bool ReloadUpdateResources(ARecord resConfig);

	virtual bool ReloadUpdateResources(NiceData &sourceParam);

	virtual void OnDBFinishLoadDataInfo(DataStream *resourcesInfoData);

	void OnDBFinishLoadOnePartData(int id, DataStream *resourcesInfoData);

	virtual void ReadySendData(AutoData sendScrData){}

public:
	int				mAllPartCount;	
	EasyString		mResourcesName;
	AString			mDateTime;
	FileDataServerTool *mpFileDataServerTool;
	//ARecord			mDataSourceConfig;
};

typedef Hand<UpdateResourcesFileTool> HandFileSendControl;
//-------------------------------------------------------------------------
// �ṩ���ع���
class FileDataServerTool //: public Base<FileDataServerTool>
{
public:
	virtual void OnLoadListTableFail(const AString &listTableName, bool bInitSqlSucceed, MySqlDBTool *sqlTool){}

public:
	FileDataServerTool(const char *szResouceListDBName)
		: mResourcesListTableName(szResouceListDBName)
	{
		AssertEx(msFileDataServerTool==NULL, "FileDataServerTool must be single");
		msFileDataServerTool = this;

		mResourcesList = tBaseTable::NewBaseTable();
		mResourcesList->SetField("INDEX", FIELD_STRING, 0);
		mResourcesList->SetField("MD5", FIELD_STRING, 1);
		mResourcesList->SetField("SIZE", FIELD_INT, 2);
		mResourcesList->SetField("DATE", FIELD_STRING, 3);
		mResourcesList->SetMainIndex("INDEX", false);
	}

	virtual ~FileDataServerTool()
	{
		Clear();
		mResourcesList._free();
		AssertEx(msFileDataServerTool!=NULL, "FileDataServerTool must be exist");
		msFileDataServerTool = NULL;
	}

	static FileDataServerTool* getSingletonPtr()
	{
		return msFileDataServerTool;
	}

	static void RegisterNetEvent(tEventCenter *pCenter);

public:
	void ReloadResourcesFileData(DataTableManager &configMgr);

	void OnResourceLoadSucceed(const char *szResName, DataStream *pResData, UpdateResourcesFileTool *pResTool);

	void UpdateResourceListMD5();

	virtual void OnLoadedResourcesList(AutoTable listTable){}

	static AString MakeResourceListMD5(AutoTable resourcesList);

	HandFileSendControl GetResourceUpdateData(const char *szResourceName)
	{
		return mFileDataList.find(szResourceName);
	}

	void Process()
	{

	}

	void Clear()
	{
		for (auto it = mFileDataList.begin(); it.have(); it.next())
		{
			it.get()._free();
		}
		mFileDataList.clear();
		mResourcesList->ClearAll();
	}

	AString GetResourcesListMD5(){ if (mResourcesList) return mResourcesList->GetTableName(); return ""; }

	virtual bool AppendUpdateList(const char *szResourceName) = 0;

	virtual void OnResourcesLoadFinish(){}

public:
	EasyHash<EasyString, HandFileSendControl>	mFileDataList;
	AutoTable									mResourcesList;
	AString										mResourcesListTableName;

	static  FileDataServerTool					*msFileDataServerTool;
};
//-------------------------------------------------------------------------
class CS_RequestResourcesList_S : public Logic::tClientEvent
{
public:
	virtual bool _DoEvent()
	{
		GetResponseEvent()->set("RES_LIST", FileDataServerTool::getSingletonPtr()->mResourcesList);
		
		Finish();
		return true;
	}
};
//-------------------------------------------------------------------------
class CS_RequestResouresInfo_S : public  FILE_RequestDataInfo_S
{
public:
	virtual bool ReadySendData(int &partCount, DSIZE &scrSize, DSIZE &zipSize, AString &checkMD5, AutoEvent &respEvt)
	{
		if (FileDataServerTool::getSingletonPtr()==NULL)
		{
			ERROR_LOG("�����ļ�����δ����");
			return false;
		}

		AString resName = get("RES_NAME");
		if (resName=="")
		{
			ERROR_LOG("������Դδ�ṩ��Դ����");
			return false;
		}

		HandFileSendControl resData = FileDataServerTool::getSingletonPtr()->GetResourceUpdateData(resName.c_str());
		if (!resData)
		{
			ERROR_LOG("��������Դ��������> [%s]", resName.c_str());
			return false;
		}

		partCount = resData->GetPartCount();
		scrSize = resData->GetScrDataSize();
		zipSize = resData->GetZipSize();
		checkMD5 = resData->GetDataMD5();

		//respEvt->set("RES_LIST", FileDataServerTool::getSingletonPtr()->GetResourcesListMD5().c_str());

		return true;
	}


};
//-------------------------------------------------------------------------
class CS_RequestResouresPartData_S : public FILE_RequestPartData_S
{
public:
	virtual Hand<FileNetTransferSendControl> GetSendControlTool() 
	{ 
		if (FileDataServerTool::getSingletonPtr()==NULL)
		{
			ERROR_LOG("�����ļ�����δ����");
			return Hand<FileNetTransferSendControl>();
		}
		AString resName = get("RES_NAME");
		if (resName=="")
		{
			ERROR_LOG("������Դδ�ṩ��Դ����");
			return Hand<FileNetTransferSendControl>();
		}

		HandFileSendControl resData = FileDataServerTool::getSingletonPtr()->GetResourceUpdateData(resName.c_str());
		if (!resData)
		{
			ERROR_LOG("��������Դ��������> [%s]", resName.c_str());
			return Hand<FileNetTransferSendControl>();
		}

		AString resMD5 = get("RES_MD5");
		if (resMD5!=resData->GetDataMD5())
		{
			ERROR_LOG("������Դ�뵱ǰ��ԴMD5�����, ��Ҫ���¿�ʼ����");
			GetResponseEvent()->set("NEED_REQUEST", true);
			return Hand<FileNetTransferSendControl>();
		}
		return resData; 
	}

};
//-------------------------------------------------------------------------
#endif //_INCLUDE_CLIENTUPDATERESOURCES_H_