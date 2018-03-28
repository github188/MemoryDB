
#include "ClientUpdateResources.h"

#include "DBManager.h"
#include "DBOperate.h"
#include "GameStruct.h"
#include "Md5Tool.h"
#include "ResoursePack.h"
#include "TableManager.h"
#include "MySqlDBSaver.h"
#include "Md5Tool.h"
#include "CommonDefine.h"

#include "PakUpdateTool.h"

bool UpdateResourcesFileTool::ReloadUpdateResources(ARecord resConfig)
{
	if (!resConfig)
	{
		NOTE_LOG("DB����Ϊ��");
		return false;
	}
	mSendFileData->clear(false);
	mZipPackData->clear(false);
	mZipPartDataList.clear();
	mAllPartCount = 0;
	mPackSize = 0;
	mPackZipSize = 0;
	// ��DB��ȡ����Ϣ
	//mLoginThread->GetDBWork()->GetRecord("t_updateresources",  "ID = -1", DBCallBack(&ClientUpdateResourcesTool::OnDBFinishLoadDataInfo, this));

	MySqlDBSaver	dbSaver;
	//ARecord resConfig;
	//if (mDataSourceConfig)
	//	resConfig = mDataSourceConfig;
	//else
	//	resConfig = configMgr.GetRecord(RUNCONFIG, "ResourcesData");
	if (resConfig)
	{
		NiceData sourceParam;
		sourceParam.set(DBIP, resConfig->get("STRING").string());
		sourceParam.set(DBPORT, (int)resConfig->get("VALUE"));
		sourceParam.set(DBUSER, resConfig->get("STRING2").string());
		sourceParam.set(DBPASSWORD, resConfig->get("STRING3").string());
		sourceParam.set(DBBASE, resConfig->get("STRING4").string());
		sourceParam.set(DBNAME, mResourcesName.c_str());
		return ReloadUpdateResources(sourceParam);
	}
	else
		ERROR_LOG("δ������Դ����Դ, ����������Դ���� > [%s] > ResourcesData", RUNCONFIG);

	return false;
}

bool UpdateResourcesFileTool::ReloadUpdateResources(NiceData &sourceParam)
{
	MySqlDBSaver	dbSaver;
	if (dbSaver.Start(sourceParam))
	{
		//mDataSource->SaveData(get("NAME").c_str(), resData->data(), resData->dataSize(), DBCallBack(&FW_RequestUploadResources_G::OnSaveResFinish, this));
		//WaitTime(20);
		//return true;
		DataBuffer infoData;
		if (dbSaver.LoadData(-1, &infoData))
		{
			OnDBFinishLoadDataInfo(&infoData);
			if (mAllPartCount>500)
				NOTE_LOG("����: ���ڵ�ȡһ���Ƚϴ����Դ >[%s] = %d", mResourcesName.c_str(), mAllPartCount)
			else
				NOTE_LOG("Now start load [%s] = %d", mResourcesName.c_str(), mAllPartCount);
			for (int i=0; i<mAllPartCount; ++i)
			{
				HandDataBuffer resData = MEM_NEW DataBuffer();
				if (!dbSaver.LoadData(i, resData.getPtr()))
				{
					ERROR_LOG("����[%s]��Դ����ʧ��, �������� [%d]", mResourcesName.c_str(), i);
					return false;
				}
				OnDBFinishLoadOnePartData(i, resData.getPtr());
			}
			return true;
		}
		else
			ERROR_LOG("����[%s]��Դ��Ϣʧ�� -1 ��������", mResourcesName.c_str());
	}
	else
	{
		ERROR_LOG("��ʼ��Դ����Դʧ��, ����ʧ��, ��ȷ�ϵ�ǰ����Ҫ���¹��� >\r\n%s", sourceParam.dump().c_str());
	}
	return false;
}

void UpdateResourcesFileTool::OnDBFinishLoadDataInfo(DataStream *resourcesInfoData)
{
	//if (bS)
	{
		//AutoData resourcesInfoData = MEM_NEW DataBuffer(size);
		//resourcesInfoData->_write((void*)szData, size);
		//if ( op->mResultRecord->get("PARTDATA", resourcesInfoData) )
		{
			resourcesInfoData->seek(0);
			CHECK_READ( resourcesInfoData, mAllPartCount );
			CHECK_READ( resourcesInfoData, mPackSize );
			CHECK_READ( resourcesInfoData, mPackZipSize );
			resourcesInfoData->readString(mCheckMD5);

			mZipPartDataList.resize(mAllPartCount);

			//if (mAllPartCount>0)
			//	_LoadPartData();
			//else
			//{
			//	OnLoadUpdateResourcesDataSucceed();
			//}
			return;
		}
		TABLE_LOG("Error: ��ȡ������Դ����ʧ��, δȡ��������Ϣ");
	}
	//else
	//{
	//	TABLE_LOG("WARN: (*����δ׼��������Դ����), ��ȡ������Դ������Ϣʧ��> [%s]", op->mErrorInfo.c_str() );
	//}	

	OnLoadUpdateResourcesDataFail();
}

void UpdateResourcesFileTool::OnDBFinishLoadOnePartData(int id, DataStream *resourcesInfoData)
{
	//if (bS)
	{
		//AutoData resourcesInfoData = MEM_NEW DataBuffer(size);
		//resourcesInfoData->_write((void*)szData, size);
		//if ( op->mResultRecord->get("PARTDATA", resourcesInfoData) )
		{
			mZipPackData->_write(resourcesInfoData->data(), resourcesInfoData->dataSize());

			//int id = TOINT(szIndex); // op->mResultRecord->get("ID");
			if (id>=mZipPartDataList.size())
			{
				TABLE_LOG("ERROR: �߼�����, ������������ȷ");
				OnLoadUpdateResourcesDataFail();
				return;
			}

			mZipPartDataList[id] = resourcesInfoData;

			if (id+1>=mAllPartCount)
			{
				//MD5 ����
				MD5 md(mZipPackData->data(), mZipPackData->dataSize());

				if (md.toString()==mCheckMD5.c_str())
					OnLoadUpdateResourcesDataSucceed();
				else
				{
					TABLE_LOG("ERROR: ��Դ����MD5У��ʧ��");
					OnLoadUpdateResourcesDataFail();
				}										
			}
			return;
		}
		TABLE_LOG("Error: ��ȡ������Դ����ʧ��, δȡ��������Ϣ");
	}
	//else
	//{
	//	TABLE_LOG("Error: ��ȡ������Դ����ʧ��> [%s]", op->mErrorInfo.c_str() );
	//}
}

void UpdateResourcesFileTool::OnLoadUpdateResourcesDataSucceed()
{
	if (mZipPackData->dataSize()<=0)
	{
		TABLE_LOG("��ǰ����Ҫ��Դ���°�");
		return;
	}

	MD5 md2(mZipPackData->data(), mZipPackData->dataSize());
	if (mCheckMD5!=md2.toString().c_str())
	{
		ERROR_LOG("У����Դѹ������MD5ʧ��");
		return;
	}

	TABLE_LOG("��ȡ������Դ���ݳɹ� > part data count [%d]", mAllPartCount);

	// ��ѹ��, ������Դ��, ȡ����Դ���е�MD5
	HandDataBuffer d = mZipPackData;
	if (d->UnZipData(mSendFileData, 0, mPackSize))
	{
		mSendFileData->seek(0);
		LOG_GREEN;
		TABLE_LOG("[%s]�ͻ�����Դ������У��ɹ�, ��������ʹ��", mResourcesName.c_str());
		LOG_WHITE;
		if (mpFileDataServerTool!=NULL)
			mpFileDataServerTool->OnResourceLoadSucceed(mResourcesName.c_str(), mSendFileData.getPtr(), this);

		return;
		/// �����Ƕ���Դ�����м��, ��ǰ���ܲ��ٷ������͵��ļ�����
		//PackHead	packInfo;
		//if (mSendFileData->_read(&packInfo, PACKHEAD_SIZE))
		//{
		//	if (mSendFileData->dataSize()>PACKHEAD_SIZE)
		//	{
		//		// �ٴζ���Դ�����ݽ���У��
		//		MD5 md(mSendFileData->data()+PACKHEAD_SIZE, mSendFileData->dataSize()-PACKHEAD_SIZE);
		//		if ( md.toString() == packInfo.mMD5 )
		//		{
		//			mResourcesPackMD5 = packInfo.mMD5;
		//			TABLE_LOG("V�ͻ�����Դ������У��ɹ�, ��������ʹ��");
		//			return;
		//		}
		//		else
		//		{
		//			TABLE_LOG("ERROR: �ͻ�����Դ������У��ʧ��, ������ʹ��");
		//		}
		//	}
		//	else
		//	{
		//		mResourcesPackMD5 = "";
		//		TABLE_LOG("WARN: ��ǰ��Դ�����޸����ļ�, ����ȷ���Ƿ���Ҫ�κθ���");
		//		return;
		//	}
		//}
	}
	else
	{
		TABLE_LOG("Error: ��Դ���ݽ�ѹ��ʧ��");
	}
	OnLoadUpdateResourcesDataFail();
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
FileDataServerTool* FileDataServerTool::msFileDataServerTool = NULL;

void FileDataServerTool::RegisterNetEvent( tEventCenter *pCenter )
{
	pCenter->RegisterEvent("CS_RequestResouresInfo", MEM_NEW EventFactory<CS_RequestResouresInfo_S>());
	pCenter->RegisterEvent("CS_RequestResouresPartData", MEM_NEW EventFactory<CS_RequestResouresPartData_S>());
	//pCenter->RegisterEvent("CS_RequestResourcesList", MEM_NEW EventFactory<CS_RequestResourcesList_S>());
	
	pCenter->RegisterEvent("CS_RequestPakDataList", MEM_NEW EventFactory<CS_RequestPakDataList>());
}

void FileDataServerTool::ReloadResourcesFileData(DataTableManager &configMgr)
{
	Clear();

	MySqlDBSaver	dbSaver;
	ARecord resConfig = configMgr.GetRecord(RUNCONFIG, "ResourcesData");
	if (resConfig)
	{
		NiceData sourceParam;
		sourceParam.set(DBIP, resConfig->get("STRING").string());
		sourceParam.set(DBPORT, (int)resConfig->get("VALUE"));
		sourceParam.set(DBUSER, resConfig->get("STRING2").string());
		sourceParam.set(DBPASSWORD, resConfig->get("STRING3").string());
		sourceParam.set(DBBASE, resConfig->get("STRING4").string());
		//sourceParam.set(DBNAME, "t_resourceslist");
		if (dbSaver.mMySqlDB.InitStart(sourceParam))
		{
			AutoTable listTable = dbSaver.mMySqlDB.LoadDBTable(mResourcesListTableName.c_str(), true);
			if (listTable)
			{
				OnLoadedResourcesList(listTable);
				for (TableIt tIt(listTable); tIt._Have(); tIt._Next())
				{					
					AString szResName = tIt.getCurrRecord()->getIndexData().string();

					if (mFileDataList.exist(szResName))
					{
						ERROR_LOG("�����б������Ҫ")
						continue;
					}

					AString date = tIt.getCurrRecord()["DATE"];
					if (!AppendUpdateList(szResName.c_str()))
						continue;

					HandFileSendControl configRes;
					if (strstr(szResName.c_str(), ".pak")!=NULL 
						&& szResName.length()>2 && (szResName.c_str()[0]!='s' || szResName.c_str()[1]!='_')
						)
					{
						NOTE_LOG("*** Ready PAK update >%s", szResName.c_str());
						configRes = MEM_NEW PakUpdateResourcesFileTool(this, szResName.c_str());
					}
					else
						configRes = MEM_NEW UpdateResourcesFileTool(this, szResName.c_str());
					configRes->mDateTime = date;

					ARecord dbConfig = configMgr.GetRecord(RUNCONFIG, "ResourcesData");		

					configRes->ReloadUpdateResources(dbConfig);
					mFileDataList.insert(szResName, configRes);
				}
				UpdateResourceListMD5();
				OnResourcesLoadFinish();
			}
			else
			{
				OnLoadListTableFail(mResourcesListTableName, true, &dbSaver.mMySqlDB);
				ERROR_LOG("����[%s]��Դ�б�ʧ��", mResourcesListTableName.c_str());
			}
		}
		else
		{
			ERROR_LOG("��ʼ��Դ����Դʧ��,MYSQL����ʧ��");
		}
	}
	else
		ERROR_LOG("δ������Դ����Դ, ���������� ResourcesData");	
}

void FileDataServerTool::UpdateResourceListMD5()
{	
	//mResourcesListMD5 = md.toString().c_str();
	mResourcesList->SetTableName(MakeResourceListMD5(mResourcesList).c_str());
}

void FileDataServerTool::OnResourceLoadSucceed( const char *szResName, DataStream *pResData, UpdateResourcesFileTool *pResTool )
{
	ARecord re = mResourcesList->CreateRecord(szResName, true);

	MD5	md((const void*)pResData->data(), pResData->dataSize());

	re->set("MD5", md.toString().c_str());
	re->set("SIZE", (int)pResTool->GetZipSize());
	re->set("DATE", pResTool->mDateTime);

	// ȫ�����غ��ٸ������е��б�MD5
	//UpdateResourceListMD5();
}

AString FileDataServerTool::MakeResourceListMD5(AutoTable resourcesList)
{
	AString infoString;
	for (TableIt tIt(resourcesList); tIt._Have(); tIt._Next())
	{
		ARecord re = tIt.getCurrRecord();
		infoString += re->getIndexData().string();
		infoString += re->get("MD5").string();
	}
	MD5 md((const void *)infoString.c_str(), infoString.length());
	return md.toString().c_str();
}

//-------------------------------------------------------------------------