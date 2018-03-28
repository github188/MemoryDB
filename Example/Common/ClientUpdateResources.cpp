
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
		NOTE_LOG("DB配置为空");
		return false;
	}
	mSendFileData->clear(false);
	mZipPackData->clear(false);
	mZipPartDataList.clear();
	mAllPartCount = 0;
	mPackSize = 0;
	mPackZipSize = 0;
	// 从DB中取出信息
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
		ERROR_LOG("未配置资源数据源, 不存在数据源配置 > [%s] > ResourcesData", RUNCONFIG);

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
				NOTE_LOG("警告: 正在调取一个比较大的资源 >[%s] = %d", mResourcesName.c_str(), mAllPartCount)
			else
				NOTE_LOG("Now start load [%s] = %d", mResourcesName.c_str(), mAllPartCount);
			for (int i=0; i<mAllPartCount; ++i)
			{
				HandDataBuffer resData = MEM_NEW DataBuffer();
				if (!dbSaver.LoadData(i, resData.getPtr()))
				{
					ERROR_LOG("加载[%s]资源数据失败, 数据索引 [%d]", mResourcesName.c_str(), i);
					return false;
				}
				OnDBFinishLoadOnePartData(i, resData.getPtr());
			}
			return true;
		}
		else
			ERROR_LOG("加载[%s]资源信息失败 -1 数据索引", mResourcesName.c_str());
	}
	else
	{
		ERROR_LOG("初始资源数据源失败, 连接失败, 请确认当前不需要更新功能 >\r\n%s", sourceParam.dump().c_str());
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
		TABLE_LOG("Error: 获取更新资源数据失败, 未取得数据信息");
	}
	//else
	//{
	//	TABLE_LOG("WARN: (*可能未准备更新资源数据), 获取更新资源数据信息失败> [%s]", op->mErrorInfo.c_str() );
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
				TABLE_LOG("ERROR: 逻辑错误, 数据索引不正确");
				OnLoadUpdateResourcesDataFail();
				return;
			}

			mZipPartDataList[id] = resourcesInfoData;

			if (id+1>=mAllPartCount)
			{
				//MD5 检验
				MD5 md(mZipPackData->data(), mZipPackData->dataSize());

				if (md.toString()==mCheckMD5.c_str())
					OnLoadUpdateResourcesDataSucceed();
				else
				{
					TABLE_LOG("ERROR: 资源数据MD5校验失败");
					OnLoadUpdateResourcesDataFail();
				}										
			}
			return;
		}
		TABLE_LOG("Error: 获取更新资源数据失败, 未取得数据信息");
	}
	//else
	//{
	//	TABLE_LOG("Error: 获取更新资源数据失败> [%s]", op->mErrorInfo.c_str() );
	//}
}

void UpdateResourcesFileTool::OnLoadUpdateResourcesDataSucceed()
{
	if (mZipPackData->dataSize()<=0)
	{
		TABLE_LOG("当前不需要资源更新包");
		return;
	}

	MD5 md2(mZipPackData->data(), mZipPackData->dataSize());
	if (mCheckMD5!=md2.toString().c_str())
	{
		ERROR_LOG("校验资源压缩数据MD5失败");
		return;
	}

	TABLE_LOG("调取更新资源数据成功 > part data count [%d]", mAllPartCount);

	// 解压缩, 生成资源包, 取出资源包中的MD5
	HandDataBuffer d = mZipPackData;
	if (d->UnZipData(mSendFileData, 0, mPackSize))
	{
		mSendFileData->seek(0);
		LOG_GREEN;
		TABLE_LOG("[%s]客户端资源包数据校验成功, 可以正常使用", mResourcesName.c_str());
		LOG_WHITE;
		if (mpFileDataServerTool!=NULL)
			mpFileDataServerTool->OnResourceLoadSucceed(mResourcesName.c_str(), mSendFileData.getPtr(), this);

		return;
		/// 以下是对资源包进行检查, 当前功能不再分析发送的文件数据
		//PackHead	packInfo;
		//if (mSendFileData->_read(&packInfo, PACKHEAD_SIZE))
		//{
		//	if (mSendFileData->dataSize()>PACKHEAD_SIZE)
		//	{
		//		// 再次对资源包数据进行校验
		//		MD5 md(mSendFileData->data()+PACKHEAD_SIZE, mSendFileData->dataSize()-PACKHEAD_SIZE);
		//		if ( md.toString() == packInfo.mMD5 )
		//		{
		//			mResourcesPackMD5 = packInfo.mMD5;
		//			TABLE_LOG("V客户端资源包数据校验成功, 可以正常使用");
		//			return;
		//		}
		//		else
		//		{
		//			TABLE_LOG("ERROR: 客户端资源包数据校验失败, 不可以使用");
		//		}
		//	}
		//	else
		//	{
		//		mResourcesPackMD5 = "";
		//		TABLE_LOG("WARN: 当前资源包内无更新文件, 请检查确认是否不需要任何更新");
		//		return;
		//	}
		//}
	}
	else
	{
		TABLE_LOG("Error: 资源数据解压缩失败");
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
						ERROR_LOG("更新列表存在重要")
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
				ERROR_LOG("加载[%s]资源列表失败", mResourcesListTableName.c_str());
			}
		}
		else
		{
			ERROR_LOG("初始资源数据源失败,MYSQL连接失败");
		}
	}
	else
		ERROR_LOG("未配置资源数据源, 不存在配置 ResourcesData");	
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

	// 全部加载后再更新所有的列表MD5
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