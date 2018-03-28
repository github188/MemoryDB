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
		// ΪPak�����е������ļ��������͹���
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
						// ���¹����в����ظ�
						if (mpFileDataServerTool->mFileDataList.exist(resName))
						{
							ERROR_LOG("���ش���, ���¹������Ѿ����ڸ�����Դ [%s]", resName);
							return;
						}
						//MD5 md(sender->mZipPackData->data(), sender->mZipPackData->dataSize());
						//sender->mCheckMD5 = md.toString().c_str();
						//sender->mPackSize = scr->dataSize();
						// NOTE: Ŀ�Ĳ���������б���
						sender->mpFileDataServerTool = NULL;
						sender->OnLoadUpdateResourcesDataSucceed();
						sender->mpFileDataServerTool = mpFileDataServerTool;
						mpFileDataServerTool->mFileDataList.insert(resName, sender);
						TableTool::green();
						//NOTE_LOG("���ط��� ResorcePack [%s] > [%s], Part count [%llu]", mResourcesName.c_str(), resName.c_str(), sender->mZipPartDataList.size());
						TableTool::white();
					}
					MD5 md(sender->mSendFileData->data(), sender->mSendFileData->dataSize());
					ARecord dataRe = mPakDataList->CreateRecord(resName, true);
					dataRe["MD5"] = md.toString().c_str();
					dataRe["SIZE"] = sender->mPackZipSize;
					NOTE_LOG("Pak data [%s] [%d] = [%s]", resName.c_str(), sender->mSendFileData->dataSize(), md.toString().c_str());
				}
				else
					ERROR_LOG("��ȡ��Դ����ʧ��[%s] form pack [%s]", resName.c_str(), mResourcesName.c_str());
			}
			NOTE_LOG("*********************************************************");
		}
		else
			ERROR_LOG("������Դ������ʧ�� >[%s]", mResourcesName.c_str());
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
