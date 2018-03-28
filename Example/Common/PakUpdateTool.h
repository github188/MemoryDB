#ifndef _INCLUDE_PAKUPDATETOOL_H_
#define _INCLUDE_PAKUPDATETOOL_H_

#include "ClientUpdateResources.h"

// 为pak 更新资源准备更新管理
// 主要是在资源包数据加载完成后, 为包中每一个数据文件准备一个发送工具
// 当客户端下载Pak文件时, 先分析需要下载的包数据文件,然后只下载需要更新的数据文件
class PakUpdateResourcesFileTool : public UpdateResourcesFileTool
{
public:
	PakUpdateResourcesFileTool(FileDataServerTool *pTool, const char *szResourceName)
		: UpdateResourcesFileTool(pTool, szResourceName), mbZip(false), mbEnrypt(false) {}

public:
	virtual void OnLoadUpdateResourcesDataSucceed();

public:
	AutoTable mPakDataList;
	bool mbZip;
	bool mbEnrypt;
	//EasyHash<AString, HandFileSendControl>	mDataSendToolList;
};

class PakDataUpdateResourcesFileTool : public UpdateResourcesFileTool
{
public:
	PakDataUpdateResourcesFileTool(FileDataServerTool *pTool, const char *szResourceName)
		: UpdateResourcesFileTool(pTool, szResourceName){}

	virtual void ReadySendData(AutoData sendScrData) override
	{
		FileNetTransferSendControl::ReadySendData(sendScrData);
	}
};

class CS_RequestPakDataList : public Logic::tClientEvent
{
public:
	virtual bool _DoEvent();
};

#endif //_INCLUDE_PAKUPDATETOOL_H_

