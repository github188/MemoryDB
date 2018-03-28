#include "PakUpdateTool.h"
#include "ResoursePack.h"
#include "Md5Tool.h"
#include "ClientUpdateResources.h"

void PakUpdateResourcesFileTool::OnLoadUpdateResourcesDataSucceed()
{
	mSendFileData.setNull();
	//FileDataServerTool *p = mpFileDataServerTool;
	//mpFileDataServerTool = NULL;
	UpdateResourcesFileTool::OnLoadUpdateResourcesDataSucceed();
	//mpFileDataServerTool = p;
	//mDataSendToolList.clear(false);

	if (mSendFileData)
	{
		//mSendFileData->seek(sizeof(int));
		//int saveSize = 0;
		//mSendFileData->read(saveSize);
		//
		//int size = mSendFileData->dataSize();
		//MD5 md(mSendFileData->data(), saveSize);
		//AString strMD5 = md.toString().c_str();
		//NOTE_LOG("[%s] [%d] save size =%d  [%s]", mResourcesName.c_str(), size, saveSize, strMD5.c_str());

		mPakDataList = tBaseTable::NewBaseTable(false);
		mPakDataList->AppendField("DATA_NAME", FIELD_STRING);
		mPakDataList->AppendField("MD5", FIELD_STRING);
		mPakDataList->AppendField("SIZE", FIELD_INT);
		// 为Pak中所有的数据文件启动发送工具
		EasyResourcesPack  pak;
		if (pak.load(mSendFileData.getPtr()))
		{
			mbZip = pak.IsZip();
			mbEnrypt = pak.IsEnrypt();
			AutoTable indexList = pak.GetResourcesIndex();
			NOTE_LOG("*********************************************************");
			for (TableIt tIt(indexList); tIt; ++tIt)
			{
				ARecord re = tIt.getCurrRecord();
				AString resName = re[0];

				AutoData data = pak.loadResource(resName.c_str());
				if (data)
				{				
					HandFileSendControl sender = MEM_NEW PakDataUpdateResourcesFileTool(mpFileDataServerTool, resName.c_str());
					sender->ReadySendData(data);
					//Hand<DataBuffer> scr = data;
					//if (scr->ZipData(sender->mZipPackData)>0)
					{
						// 更新管理中不能重复
						if (mpFileDataServerTool->mFileDataList.exist(resName))
						{
							ERROR_LOG("严重错误, 更新管理中已经存在更新资源 [%s]", resName);
							return;
						}
						//MD5 md(sender->mZipPackData->data(), sender->mZipPackData->dataSize());
						//sender->mCheckMD5 = md.toString().c_str();
						//sender->mPackSize = scr->dataSize();
						// NOTE: 目的不加入更新列表中
						sender->mpFileDataServerTool = NULL;
						sender->OnLoadUpdateResourcesDataSucceed();
						sender->mpFileDataServerTool = mpFileDataServerTool;
						mpFileDataServerTool->mFileDataList.insert(resName, sender);
						TableTool::green();
						//NOTE_LOG("下载服务 ResorcePack [%s] > [%s], Part count [%llu]", mResourcesName.c_str(), resName.c_str(), sender->mZipPartDataList.size());
						TableTool::white();
					}
					MD5 md(sender->mSendFileData->data(), sender->mSendFileData->dataSize());
					ARecord dataRe = mPakDataList->CreateRecord(resName, true);
					dataRe["MD5"] = md.toString().c_str();
					dataRe["SIZE"] = sender->mPackZipSize;
					NOTE_LOG("Pak data [%s] [%d] = [%s]", resName.c_str(), sender->mSendFileData->dataSize(), md.toString().c_str());
				}
				else
					ERROR_LOG("获取资源数据失败[%s] form pack [%s]", resName.c_str(), mResourcesName.c_str());
			}
			NOTE_LOG("*********************************************************");
		}
		else
			ERROR_LOG("更新资源包加载失败 >[%s]", mResourcesName.c_str());
	}
	else
		mPakDataList.setNull();
}

bool CS_RequestPakDataList::_DoEvent()
{
	AString resName = get("RES_NAME");
	Hand<PakUpdateResourcesFileTool> tool = FileDataServerTool::getSingletonPtr()->mFileDataList.find(resName);
	if (tool)
	{
		GetResponseEvent()->set("DATA_LIST", tool->mPakDataList);
		GetResponseEvent()["IS_ZIP"] = tool->mbZip;
		GetResponseEvent()["IS_ENRYPT"] = tool->mbEnrypt;
	}

	Finish();
	return true;
}
