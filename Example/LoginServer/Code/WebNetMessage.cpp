#include "WebNetMessage.h"
#include "GameDefine.h"
#include "WebPlayer.h"
#include "BigMsgEvent.h"


void WEBMsg::Register(WebGameNet *pWebNet)
{
	pWebNet->RegisterMsg(WEB_CL_RequestLoginGame, MEM_NEW EventFactory<CS_ReuestLoginGame>());
	pWebNet->RegisterMsg(CL_REGISTER_ACCOUNT, MEM_NEW EventFactory<CL_RequestRegisterAccount>());
	//pWebNet->RegisterMsg(CL_REQUEST_UPLOAD_PHOTO, MEM_NEW EventFactory<CL_RequestUploadPhoto>());
	pWebNet->RegisterMsg(CL_REQUEST_LOAD_PHOTO, MEM_NEW EventFactory<CL_RequestLoadPhotoData>() );

	//pWebNet->RegisterMsg(CL_REQUEST_LOAD_SELF_PHOTO, MEM_NEW EventFactory<CL_RequestLoadSelfPhotoData>() );
    //WebNet->RegisterMsg(CL_REQUEST_LOAD_HEAD_PHOTO, MEM_NEW EventFactory<CL_RequestLoadSelfPhotoData>() );

	pWebNet->RegisterMsg(CL_REQUEST_DB_DATA, MEM_NEW EventFactory<CL_RequestDBData>());
	pWebNet->RegisterMsg(CL_REQUEST_UPDATE_DB_DATA, MEM_NEW EventFactory<CL_RequestUpdateDBData>());

	pWebNet->RegisterMsg(CL_REQEUST_SAVE_NEWSFEED, MEM_NEW EventFactory<CL_RequestSaveNewsFeed>());
	pWebNet->RegisterMsg(CL_LOAD_NEWSFEED, MEM_NEW EventFactory<CL_LoadNewsFeed>());
	pWebNet->RegisterMsg(CL_LOAD_BIG_PHOTO, MEM_NEW EventFactory<CL_LoadNewsFeedLoadBigPhoto>());
}

void CL_RequestRegisterAccount::OnCreateAccountFinish( DBOperate *pDB, bool bSu )
{
	if (bSu)
	{
		if ((int)pDB->mResultNiceData["RESULT"]==eCreateAccountSucceed)
		{
			Hand<WebPlayer> player = mNetConnect;
			player->mDBID = (int)pDB->mResultNiceData["DBID"];
			GetResponseEvent()["DBID"] = player->mDBID;
			GetResponseEvent()["RESULT"] = true;
		}
		else
		{
			GetResponseEvent()["ERROR"] = (int)pDB->mResultNiceData["RESULT"];
		}			
	}
	else
	{
		GetResponseEvent()["ERROR"] = eDBQueryError;
		ERROR_LOG("查询帐号失败");
	}
	Finish();
}

void CS_ReuestLoginGame::OnCheckAccountFinish( DBOperate *pDB, bool bSu )
{
	if (bSu)
	{
		if ((int)pDB->mResultNiceData["RESULT"]==eLoginSucceed)
		{
			Hand<WebPlayer> player = mNetConnect;
			player->mDBID = (int)pDB->mResultNiceData["DBID"];
			GetResponseEvent()["DBID"] = player->mDBID;
			GetResponseEvent()["RESULT"] = true;
			GetResponseEvent()["PHOTO_MD5"] = pDB->mResultNiceData["PHOTO_MD5"].string();
			GetResponseEvent()["SYNC_MD5_NICE"] = (tNiceData*)pDB->mResultNiceData["SYNC_MD5_NICE"];
		}
		else
			GetResponseEvent()["ERROR"] = (int)pDB->mResultNiceData["RESULT"];
	}
	else
	{
		GetResponseEvent()["ERROR"] = eDBQueryError;
		ERROR_LOG("查询帐号失败");
	}
	Finish();
}

//bool CL_RequestUploadPhoto::_DoEvent()
//{
//	Hand<WebPlayer> player = mNetConnect;
//	if (player->mDBID<=0)
//	{
//		ERROR_LOG("未登陆");
//		GetResponseEvent()["RESULT"] = eNoLoginState;
//		Finish();
//		return true;
//	}
//	AutoData photoData = get("DATA");
//
//	AString md5 = get("MD5");
//	
//	if (photoData && photoData->dataSize()>0)
//	{
//		//AutoData pData = MEM_NEW DataBuffer(photoData.length()+1);
//		//pData->_write((void*)photoData.c_str(), photoData.length()+1);
//
//		AutoNice d = MEM_NEW NiceData();
//		d["DATA"] = photoData.getPtr();			
//		d["MD5"] = md5;
//		GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CL_RequestUploadPhoto::OnUploadFinish, this), DB_USER_TABLE, STRING(player->mDBID), "f_upload_photo", d);
//		return true;
//	}
//	GetResponseEvent()["RESULT"] = eNullData;
//	Finish();
//	return true;
//}


bool CL_RequestLoadPhotoData::_DoEvent()
{
	Hand<WebPlayer> player = mNetConnect;
	AutoNice d = MEM_NEW NiceData();
	d["NOW_ID"] = player->mNowShowID;
	GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CL_RequestLoadPhotoData::OnLoadFinish, this), DB_USER_TABLE, STRING(player->mDBID), "f_load_photo", AutoNice());
	return true;
}

void CL_RequestLoadPhotoData::SendResponce( AutoEvent hRespEvent )
{
	if (mNetConnect)
	{
		AutoData d = (DataStream*)GetResponseEvent()["DATA"];
		if (d &&d->dataSize()>10*1024)
		{			
			Hand<WEB_SendDataPartEvent> evt = GetEventCenter()->StartEvent(WEB_SEND_BIG_MSG);
			evt->mNetConnect = mNetConnect;
			evt->SendBigEvent(hRespEvent);
		}
		else
			mNetConnect->SendEvent(hRespEvent.getPtr());
	}
	//if (mNetConnect)
	//{
	//	Hand<WEB_SendDataPartEvent> evt = GetEventCenter()->StartEvent(WEB_SEND_BIG_MSG);
	//	evt->mNetConnect = mNetConnect;
	//	evt->SendBigEvent(hRespEvent);
	//}
}

void CL_RequestLoadPhotoData::OnLoadFinish( DBOperate *pDB, bool bSu )
{
	AutoData d = (DataStream*)pDB->mResultNiceData["DATA"];
	if (d)
	{
		GetResponseEvent()["DATA"] = d.getPtr();
		Hand<WebPlayer> player = mNetConnect;
		if (player)
			player->mNowShowID = pDB->mResultNiceData["NOW_ID"];
	}
	Finish();
}

//bool CL_RequestLoadSelfPhotoData::_DoEvent()
//{
//	Hand<WebPlayer> player = mNetConnect;
//	AutoNice d = MEM_NEW NiceData();
//	d["SELF"] = true;
//	GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CL_RequestLoadSelfPhotoData::OnLoadFinish, this), DB_USER_TABLE, STRING(player->mDBID), "f_load_photo", d);
//	return true;
//}
//
//void CL_RequestLoadSelfPhotoData::SendResponce( AutoEvent hRespEvent )
//{
//	if (mNetConnect)
//	{
//		Hand<WEB_SendDataPartEvent> evt = GetEventCenter()->StartEvent(WEB_SEND_BIG_MSG);
//		evt->mNetConnect = mNetConnect;
//		evt->SendBigEvent(hRespEvent);
//	}
//}

bool CL_RequestDBData::_DoEvent()
{
	Hand<WebPlayer> player = mNetConnect;
	if (player->mDBID>0)
	{
		AString field = get("FIELD");
		AString key = get("KEY");
		AutoNice d = MEM_NEW NiceData();
		d["FIELD"] = field;
		d["KEY"] = key;
		GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CL_RequestDBData::OnLoad, this), DB_USER_TABLE, STRING(player->mDBID), "f_load_db_data", d);
		return true;
	}
	Finish();
	return true;
}

void CL_RequestDBData::SendResponce( AutoEvent hRespEvent )
{
	NOTE_LOG("*** Response %s_%s data:\r\n%s", get("FIELD").string().c_str(), get("KEY").string().c_str(), hRespEvent->GetData().dump().c_str());

	if (mNetConnect)
	{
		AutoData d = (DataStream*)GetResponseEvent()["VALUE"];
		if (d &&d->dataSize()>10*1024)
		{			
			Hand<WEB_SendDataPartEvent> evt = GetEventCenter()->StartEvent(WEB_SEND_BIG_MSG);
			evt->mNetConnect = mNetConnect;
			evt->SendBigEvent(hRespEvent);
		}
		else
			mNetConnect->SendEvent(hRespEvent.getPtr());
	}
}

bool CL_RequestUpdateDBData::_DoEvent()
{
	Hand<WebPlayer> player = mNetConnect;
	if (player->mDBID>0)
	{
		AString field = get("FIELD");
		AString key = get("KEY");
		AutoNice d = MEM_NEW NiceData();
		d["FIELD"] = field;
		d["KEY"] = key;
		d["VALUE"].setData(get("VALUE"));
        d["MD5"] = get("MD5").string();
		GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CL_RequestUpdateDBData::OnSaveFinish, this), DB_USER_TABLE, STRING(player->mDBID), "f_update_db_data", d);
		return true;
	}
	Finish();
	return true;
}

bool CL_RequestSaveNewsFeed::_DoEvent()
{
	Hand<WebPlayer> player = mNetConnect;
	if (player->mDBID>0)
	{
		AString text = get("TEXT");
		AutoData picData = get("PIC_DATA");
		if (text.length()<=0 && (!picData || picData->dataSize()<=0))
		{
			ERROR_LOG("%d Save news feed fail, data is null", player->mDBID);
			Finish();
			return true;
		}

		AutoNice d = MEM_NEW NiceData();
		d["TEXT"] = text;
		d["PIC_DATA"] = picData.getPtr();
		d["SMALL_DATA"].setData(get("SMALL_DATA"));
		GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CL_RequestSaveNewsFeed::OnSaveFinish, this), DB_USER_TABLE, STRING(player->mDBID), "f_save_news_feed", d);
		return true;
	}
	Finish();
	return true;
}

void CL_LoadNewsFeed::SendResponce( AutoEvent hRespEvent )
{
	NOTE_LOG("*** Response %s_%s data:\r\n%s", GetEventName(), hRespEvent->GetData().dump().c_str());

	if (mNetConnect)
	{
		AutoData d = (DataStream*)GetResponseEvent()["PIC_DATA"];
		if (d &&d->dataSize()>10*1024)
		{			
			Hand<WEB_SendDataPartEvent> evt = GetEventCenter()->StartEvent(WEB_SEND_BIG_MSG);
			evt->mNetConnect = mNetConnect;
			evt->SendBigEvent(hRespEvent);
		}
		else
			mNetConnect->SendEvent(hRespEvent.getPtr());
	}
}

bool CL_LoadNewsFeed::_DoEvent()
{
	Hand<WebPlayer> player = mNetConnect;
	AutoNice d = MEM_NEW NiceData();
	d["REQUEST_ID"] = (int)get("REQUEST_ID");
	d["LAST_ID"] = (int)get("LAST_ID");
	GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CL_LoadNewsFeed::OnLoad, this), DB_USER_TABLE, STRING(player->mDBID), "f_load_news_feed", d);
	return true;
}

bool CL_LoadNewsFeedLoadBigPhoto::_DoEvent()
{
	Hand<WebPlayer> player = mNetConnect;
	if (player && player->mDBID>0)
	{
		AutoNice d = MEM_NEW NiceData();
		d->append(GetData(), true);		
		GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CL_LoadNewsFeedLoadBigPhoto::OnLoad, this), DB_NEWS_FEED_TALBE, get("NEWS_ID").string().c_str(), "f_load_news_feed_big_pic", d);
		return true;
	}
	Finish();
	return true;
}

void CL_LoadNewsFeedLoadBigPhoto::SendResponce( AutoEvent hRespEvent )
{
	if (mNetConnect)
	{		
		Hand<WEB_SendDataPartEvent> evt = GetEventCenter()->StartEvent(WEB_SEND_BIG_MSG);
		evt->mNetConnect = mNetConnect;
		evt->SendBigEvent(hRespEvent);
	}
}
