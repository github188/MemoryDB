/********************************************************************
	created:	2014/10/11
	created:	11:10:2014   18:00
	filename: 	D:\Code\MemoryDBServer\Server\GameServer\Server\ClientUpdateResources.h
	file path:	D:\Code\MemoryDBServer\Server\GameServer\Server
	file base:	ClientUpdateResources
	file ext:	h
	author:		Yang Wenge
	
	purpose:	文件下载服务
				为每个需要下载的文件创建 UpdateResourcesFileTool, 保存在MAP列表中
				1 客户端请求信息, 指定资源文件名, 根据名称得到 FileTool信息
				2 请求数据块消息, 也需要指定资源文件名, 获取到 FileTool 后, 进行回复数据
				  为了支持动态更新, 还需要提供资源的MD5校验码, 保证资源数据的一致性!
				3 用于检验MD5是指压缩数据的MD5 CheckMD5
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
		NOTE_LOG("Error: XXX 获取更新资源数据失败");
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
// 提供下载管理
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
			ERROR_LOG("下载文件管理还未创建");
			return false;
		}

		AString resName = get("RES_NAME");
		if (resName=="")
		{
			ERROR_LOG("请求资源未提供资源名称");
			return false;
		}

		HandFileSendControl resData = FileDataServerTool::getSingletonPtr()->GetResourceUpdateData(resName.c_str());
		if (!resData)
		{
			ERROR_LOG("不存在资源更新数据> [%s]", resName.c_str());
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
			ERROR_LOG("下载文件管理还未创建");
			return Hand<FileNetTransferSendControl>();
		}
		AString resName = get("RES_NAME");
		if (resName=="")
		{
			ERROR_LOG("请求资源未提供资源名称");
			return Hand<FileNetTransferSendControl>();
		}

		HandFileSendControl resData = FileDataServerTool::getSingletonPtr()->GetResourceUpdateData(resName.c_str());
		if (!resData)
		{
			ERROR_LOG("不存在资源更新数据> [%s]", resName.c_str());
			return Hand<FileNetTransferSendControl>();
		}

		AString resMD5 = get("RES_MD5");
		if (resMD5!=resData->GetDataMD5())
		{
			ERROR_LOG("请求资源与当前资源MD5不相符, 需要重新开始请求");
			GetResponseEvent()->set("NEED_REQUEST", true);
			return Hand<FileNetTransferSendControl>();
		}
		return resData; 
	}

};
//-------------------------------------------------------------------------
#endif //_INCLUDE_CLIENTUPDATERESOURCES_H_