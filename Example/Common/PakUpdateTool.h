#ifndef _INCLUDE_PAKUPDATETOOL_H_
#define _INCLUDE_PAKUPDATETOOL_H_

#include "ClientUpdateResources.h"

// Ϊpak ������Դ׼�����¹���
// ��Ҫ������Դ�����ݼ�����ɺ�, Ϊ����ÿһ�������ļ�׼��һ�����͹���
// ���ͻ�������Pak�ļ�ʱ, �ȷ�����Ҫ���صİ������ļ�,Ȼ��ֻ������Ҫ���µ������ļ�
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

