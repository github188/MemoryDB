#include "DBNetPacket.h"
#include "MemoryDBNode.h"

#include "DBServerOperate.h"
//-------------------------------------------------------------------------*/
void RequestDBOperateEvent::InitReadyForNet(Hand<tBaseEventNet> net)
{
	if (net->GetNetProtocol()==NULL)
		net->SetNetProtocol(MEM_NEW DBNetProtocol());
	else
		DBNetProtocol::RegisterPacket(net->GetNetProtocol());

	net->GetEventCenter()->RegisterEvent("RequestDBOperateEvent", MEM_NEW EventFactory<RequestDBOperateEvent, false>(false));
	net->GetEventCenter()->RegisterEvent("RequestDBSaveEvent", MEM_NEW EventFactory<RequestDBSaveEvent, false>(false));
}
//-------------------------------------------------------------------------*/
bool RequestDBOperateEvent::Send(int nType /* = 0 */, int nTarget /* = 0 */ )
{
	Auto<RequestDBOperatePacket> request = mNetConnect->GetNetHandle()->GetNetProtocol()->CreatePacket(PACKET_REQUEST_DBOPERATE);
	return request->SendRequestEvent(GetSelf(), mNetConnect.getPtr());
}
//-------------------------------------------------------------------------*/
bool RequestDBOperatePacket::SendRequestEvent(Hand<RequestDBOperateEvent> requstEvt, tNetConnect *pConn)
{
	if (!requstEvt)
	{
		NOTE_LOG("ERROR: SetRequestEvent Need RequestDBOperate Event param");
		return false;
	}
	if (!mData)
		mData = MEM_NEW DataBuffer(1024);
	else
		mData->clear(false);

	mData->write((int)-1);	// WEB: ��Ҫ����
	mData->write(requstEvt->GetEventID());
	mData->writeString(requstEvt->mTableIndex);
	mData->writeString(requstEvt->mRecordKey);
	mData->writeString(requstEvt->mFunctionName);
	mData->write(requstEvt->mbNeedResponse);	// WEB: ��Ҫ����
	if (requstEvt->mParamData && !requstEvt->mParamData->serialize(mData.getPtr(), false))
	{
		NOTE_LOG("ERROR: RequestDBOperatePacket::SetSendRequestEvent save param NiceData fail");
		return false;
	}
	mData->setDataSize(mData->tell());
	return pConn->Send(this, false);
}

UINT RequestDBOperatePacket::Execute( tNetConnect* pConnect )
{
	EVENT_ID eventID = 0;
	AString tableIndex;
	AString recordKey;
	AString funName;

	mData->seek(0);

	if (!mData->read(mUserConnectID))
		goto error;

	if (!mData->read(eventID))
		goto error;

	if (!mData->readString(tableIndex))
		goto error;

	if (!mData->readString(recordKey))
		goto error;

	MemoryDBNode *pDBNode = NULL;
	if (mUserConnectID<0)
		pDBNode = dynamic_cast<MemoryDBNode*>(DB_Responce::_GetDB(pConnect->GetSelf()));
	else
	{
		NodeServerConnect *pConn = dynamic_cast<NodeServerConnect*>(pConnect);
		if (pConn!=NULL && pConn->mNetNodeConnectData)
		{
			Hand<DBMeshedNodeNet> net = pConn->mNetNodeConnectData->mMeshedNet;
			if (net)
				pDBNode = net->mpDBNode;
		}
	}
	
	if (pDBNode==NULL)
		goto error;

	bool bInThis = (tableIndex==""
		|| recordKey == ""
		|| (pDBNode->CheckKeyInThisNodeRange(tableIndex.c_str(), recordKey.c_str()))
		);
	
	// NOTE: ת�������� mUserConnectID<0 ʱ, ��ʾδ��ת����, ��ֻ׼ת��һ��
	if (!bInThis && mUserConnectID<0 )
	{	
		HandConnect conn = pDBNode->FindNodeByKey(tableIndex.c_str(), recordKey.c_str());
		if (conn)
		{
			// NOTE: ��USER ������ID д��
			mData->seek(0);
			mData->write((int)pConnect->GetNetID());		
			NOTE_LOG("DB �������� NODE [%s:%d] ����", conn->GetIp(), conn->GetPort());
			conn->Send(this, false);
			return 0;
		}
		NOTE_LOG("ERROR: No exist db [%s] >%s Node", tableIndex.c_str(), recordKey.c_str());
	}

	if (!mData->readString(funName))
		goto error;

	if (!mData->read(mbNeedResponse))
		goto error;

	// ����δ�ṩ������ֻ���Իָ�
	mParamData->initData();
	mParamData->restore(mData.getPtr());

	if (bInThis)
		//tableIndex==""
		//|| recordKey == ""
		//|| (pDBNode->CheckKeyInThisNodeRange(tableIndex.c_str(), recordKey.c_str()))
		//)
	{		
		eDBResultType result = eNoneError;
		AutoOpereate op = pDBNode->RunOperate(pConnect, funName.c_str(), tableIndex.c_str(), recordKey.c_str(), mParamData, result);

		if (op && result==eWaitFinish)
		{
			mID = eventID;
			mWaitOperate = op;
			mNetConnect = pConnect;
			if (!mCallBackTool)
				mCallBackTool = MEM_NEW CallTool(this);
			else
				mCallBackTool->mpPacketOwner = this;
			op->mCallBack.setFunction(&CallTool::_CallBack, mCallBackTool.getPtr());
		}
		else
		{
			// NOTE: �������Ҫ�ظ�, ��ִ�гɹ���, ���ٻظ���ʹ�ö�
			if (!mbNeedResponse && result == eNoneError)
				return 0;

			Auto<ResponseDBOperatePacket> resp = pConnect->GetNetHandle()->GetNetProtocol()->CreatePacket(PACKET_RESPONSE_DBOPERATE);
			resp->SetResult( eventID, op, result, mUserConnectID);
			pConnect->Send(resp.getPtr(), false);
		}
	}
	else
	{
		//int result = eNoneError;
		//HandConnect conn = pDBNode->FindNodeByKey(tableIndex.c_str(), recordKey.c_str());
		//if (conn)
		//{
		//	NOTE_LOG("ERROR: RequestDBOperatePacket::Execute δʵ�� RequestDBOperatePacket ��NODE���� ");
		//	result = eDBOperateResultFail;
		//}
		//else
		//{
		//	result = eDBNodeNoExist;
		//}
		Auto<ResponseDBOperatePacket> resp = pConnect->GetNetHandle()->GetNetProtocol()->CreatePacket(PACKET_RESPONSE_DBOPERATE);
		resp->SetResult(eventID, AutoOpereate(), eDBNodeNoExist, mUserConnectID);
		pConnect->Send(resp.getPtr(), false);
	}
	return 0;

error:
	ERROR_LOG("RequestDBOperatePacket::Execute Read param fail");
	return -1;
}

void RequestDBOperatePacket::OnExeFinish( AutoNice resultData, bool bSu )
{
	// NOTE: �������Ҫ�ظ�, ��ִ�гɹ���, ���ٻظ���ʹ�ö�
	if (!mbNeedResponse && bSu)
		return;

	if (mNetConnect)
	{
		Auto<ResponseDBOperatePacket> resp = mNetConnect->GetNetHandle()->GetNetProtocol()->CreatePacket(PACKET_RESPONSE_DBOPERATE);
		resp->SetResult( mID, mWaitOperate, bSu ? eNoneError : eDBOperateResultFail, mUserConnectID);
		mNetConnect->Send(resp.getPtr(), false);
	}
	else
	{
		WARN_LOG("[%s] DB������ɺ��������ͷ�", bSu ? "�ɹ�":"ʧ��");
		TABLE_LOG(resultData->dump().c_str());
	}
}

//-------------------------------------------------------------------------*/
void ResponseDBOperatePacket::SetResult(EVENT_ID eventID, AutoOpereate op, int reslut, int userConnectID)
{
	if (mData)
		mData->clear(false);
	else
		mData = MEM_NEW DataBuffer(1024);
	mData->write(userConnectID);
	mData->write(eventID);
	mData->write(reslut);
	if (reslut==eWaitFinish)
	{
		AssertEx(0, "Ŀǰ��δ����첽������Ҫ�����ڶ�NODE����");
		NOTE_LOG("Ŀǰ��δ����첽������Ҫ�����ڶ�NODE����");
		mData->writeString("Ŀǰ��δ����첽������Ҫ�����ڶ�NODE����");
	}
	else if (op)
	{						
		mData->writeString(op->mErrorInfo);
		if (!op->mResultData->serialize(mData.getPtr(), false))
		{
			ERROR_LOG("ResponseDBOperatePacket::SetResult ���������������ʧ��");
		}
	}
	else
		mData->writeString("ִ��ʧ��");
}

//-------------------------------------------------------------------------*/
UINT ResponseDBOperatePacket::Execute( tNetConnect* pConnect )
{
	mData->seek(0);

	int userConnectID = 0;

	EVENT_ID eventID;
	int result;
	AString	resultInfo;

	if (!mData->read(userConnectID))
		return -1;

	// NOTE: userConnectID>=0, ��ʾ��Ҫת��, ת��ǰ, �����ID, �����ٴα�ת��ʧ��
	if (userConnectID>=0)
	{
		NodeRequestConnect *pConn = dynamic_cast<NodeRequestConnect*>(pConnect);
		if (pConn!=NULL && pConn->mNetNodeConnectData)
		{
			Hand<DBMeshedNodeNet> net = pConn->mNetNodeConnectData->mMeshedNet;
			if (net)
			{
				if (userConnectID<net->mpDBNode->mUserConnectList.size())
				{
					HandConnect conn = net->mpDBNode->mUserConnectList[userConnectID];
					if (conn && conn->GetNetID()==userConnectID)
					{
						// NOTE: ���ת������ID, �����ٴα�ת��ʧ��
						mData->seek(0);
						mData->write(-1);
						conn->Send(this, false);
						return 0;
					}
				}
			}
		}
		NOTE_LOG("ERROR: ԭʹ�ö�����δ�ҵ���DB��������޷��ʹ�");
		return 0;
	}

	if (!mData->read(eventID))
		return -1;
	if (!mData->read(result))
		return -1;
	
	mData->readString(resultInfo);

	// ִ�д���ᣬӦ�ò��ظ�ִ������
	mResultData->initData();
	mResultData->restore(mData.getPtr());
	
	// ֱ��ʹ�õȴ����¼�����
	ResponseEventFactory *f = dynamic_cast<ResponseEventFactory*>(pConnect->GetNetHandle()->GetEventCenter()->GetResponseEventFactory().getPtr());
	Hand<tServerEvent> hWaitEvt = f->FindServerEvent(eventID); 
	if (hWaitEvt)
	{
		Hand<RequestDBOperateEvent> request = hWaitEvt;
		if (request)
		{		
			if (request->getFinished() )
				request->Log("Warn: ���յ��ظ�ʱ, �Ѿ����");
			else
			{
				request->OnResponse(result, mResultData, resultInfo);
			}
		}
		// �Ż�: ֱ��������¼�, �������¼������á� �����ϵ�ǰ����ʱ���б���,�����ʱ����ȴ�ʱ�䳬ʱ�����Ƴ�����
		// NOTE: ����ִ�в����¼�ֱ���ͷţ��ڲ�ʹ�ù����в������ٱ�ʹ��
		hWaitEvt._free();
	}
	else
	{
		WARN_LOG("�ظ�ʱ, �ȴ��¼����ٴ��� [%llu]", eventID);
	}
	return 0;
}

//-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*/

bool RequestDBSavePacket::SendRequestEvent( Hand<RequestDBSaveEvent> requstEvt, tNetConnect *pConn )
{
	if (mData)
		mData->clear(false);				
	else
		mData = MEM_NEW DataBuffer(1024);

	mData->write(requstEvt->GetEventID());
	mData->writeString(requstEvt->mTableIndex);
	mData->writeString(requstEvt->mRecordKey);
	mData->write(requstEvt->mResultRecord->getField()->GetCheckCode());
	mData->write(requstEvt->mbNewInsert);
	mData->write(requstEvt->mbReplace);
	mData->write(requstEvt->mbGrowthKey);
	mData->write(requstEvt->mbNeedResponse);

	bool bSave =false;
	AutoData d;
	if (requstEvt->mbNewInsert)
	{
		d = mSaveData;
		d->clear(false);
		bSave = requstEvt->mResultRecord && requstEvt->mResultRecord->saveData(d.getPtr());
	}
	else
	{
		d = mUpdateData;
		d->clear(false);
		bSave = requstEvt->mResultRecord && requstEvt->mResultRecord->saveUpdateData(d.getPtr());
	}
	if (bSave)
	{
		mData->writeData(d.getPtr(), d->tell());
		mData->setDataSize(mData->tell());
		if (pConn->Send(this, false))
		{
			requstEvt->mResultRecord->FullAllUpdate(false);
			return true;
		}
	}
	else
		NOTE_LOG("ERROR: RequestDBSavePacket::SetSendRequestEvent save mResultRecord fail");

	return false;
}

UINT RequestDBSavePacket::Execute( tNetConnect* pConnect )
{
	MemoryDBNode *pDBNode = dynamic_cast<MemoryDBNode*>(DB_Responce::_GetDB(pConnect->GetSelf()));

	EVENT_ID eventID = 0;
	AString recordKey;
	if (Do(pDBNode, eventID, recordKey))
	{
		Auto<ResponseDBSavePacket> resp = pConnect->GetNetHandle()->GetNetProtocol()->CreatePacket(PACKET_RESPONSE_DBSAVE);
		resp->SetResult( eventID, mResult, recordKey);
		pConnect->Send(resp.getPtr(), false);
	}
	return 0;
}

bool RequestDBSavePacket::Do(MemoryDBNode *pDBNode, EVENT_ID &eventID, AString &recordKey )
{
	mResult = eNoneError;

	AString tableIndex;
	//AString recordKey;
	int checkCode = 0;
	bool bNewInsert = false;
	bool bReplace = false;
	bool bGrowthKey = false;
	bool bNeedResponse = false;

	mData->seek(0);

	if (!mData->read(eventID))
		goto error;

	if (!mData->readString(tableIndex))
		goto error;

	if (!mData->readString(recordKey))
		goto error;
	if (!mData->read(checkCode))
		goto error;

	if (!mData->read(bNewInsert))
		goto error;
	if (!mData->read(bReplace))
		goto error;
	if (!mData->read(bGrowthKey))
		goto error;
	if (!mData->read(bNeedResponse))
		goto error;

	if (!mData->readData(mSaveData.getPtr()))
		goto error;


	if ( bGrowthKey
		|| tableIndex==""
		|| (pDBNode->CheckKeyInThisNodeRange(tableIndex.c_str(), recordKey.c_str()))
		)
	{		

	}
	else
	{
		HandConnect conn = pDBNode->FindNodeByKey(tableIndex.c_str(), recordKey.c_str());
		if (conn)
		{
			NOTE_LOG("ERROR: RequestDBSavePacket::Execute δʵ�� RequestDBSavePacket ��NODE���� ");
			mResult = eRecordSaveToUpdateDataFail;
		}
		else
		{
			mResult = eDBNodeNoExist;
		}
		return true;
	}

	{
		ABaseTable t = pDBNode->GetTable(tableIndex.c_str());
		if (!t)
		{
			mResult = eTableNoExist;			
			return true;
		}
		if (!t->GetField()->CheckSame(checkCode))
		{
			//ReplyError("�ֶ�У��ʧ�� now [%d] of [%d]", checkCode, t->GetField()->GetCheckCode());
			mResult = eFieldCheckFail;			
			return true;
		}

		if (bNewInsert && bGrowthKey)
		{			
			AutoRecord re;
			mResult = pDBNode->_InsertGrowthRecord(tableIndex.c_str(), mSaveData.getPtr(), re);
			if (re)
				recordKey = re[0];
			return (bNeedResponse || mResult!=eNoneError);				
		}
		else
		{
			if (recordKey=="")
			{
				ERROR_LOG("ERROR: δ�ṩ��ȷ�ļ�¼KEY");
				mResult = eRecordKeyError;				
				return true;
			}

			mResult = pDBNode->_SaveRecord(tableIndex.c_str(), recordKey.c_str(), mSaveData.getPtr(), bReplace, !bNewInsert);
			return (bNeedResponse || mResult!=eNoneError);		
		}		
	}

error:
	NOTE_LOG("RequestDBSavePacket::Execute Read param fail");
	mResult = eDBOperateParamError;
	return true;
}

//-------------------------------------------------------------------------*/

UINT ResponseDBSavePacket::Execute( tNetConnect* pConnect )
{
	mData->seek(0);

	EVENT_ID eventID;
	int result;
	if (!mData->read(eventID))
		return -1;
	if (!mData->read(result))
		return -1;

	// ֱ��ʹ�õȴ����¼�����
	ResponseEventFactory *f = dynamic_cast<ResponseEventFactory*>(pConnect->GetNetHandle()->GetEventCenter()->GetResponseEventFactory().getPtr());
	Hand<tServerEvent> hWaitEvt = f->FindServerEvent(eventID); 
	if (hWaitEvt)
	{
		Hand<RequestDBSaveEvent> request = hWaitEvt;
		if (request)
		{					
			if (!mData->readString(request->mRecordKey))
				return -1;
			if (request->getFinished() )
				request->Log("Warn: ���յ��ظ�ʱ, �Ѿ����");
			else
			{
				request->OnResponse(result);
			}
		}
		// �Ż�: ֱ��������¼�, �������¼������á� �����ϵ�ǰ����ʱ���б���,�����ʱ����ȴ�ʱ�䳬ʱ�����Ƴ�����
		// NOTE: ����ִ�в����¼�ֱ���ͷţ��ڲ�ʹ�ù����в������ٱ�ʹ��
		hWaitEvt._free();
	}
	else
	{
		//WARN_LOG("�ظ�ʱ, �ȴ��¼����ٴ��� [%llu]", eventID);
		if (result!=eNoneError)
			NOTE_LOG("ERROR: ResponseDBSavePacket fail >[%d]", result);
	}
	return 0;
}
//-------------------------------------------------------------------------*/
bool RequestDBSaveEvent::Send( int nType /* = 0 */, int nTarget /* = 0 */ )
{
	if (!mResultRecord)
	{
		mResultType = eNoSetRequestRecord;
		mErrorInfo = "δ������Ҫ���µļ�¼";
		Finish();
		return false;
	}

	if (!mResultRecord->NeedUpdate())
	{
		mResultType = eRecordNotNeedUpdate;
		mErrorInfo = "��¼���ݲ���Ҫ����";
		Finish();
		return false;
	}

	if (!mbGrowthKey && mResultRecord->getField()->getFieldType(0)!=FIELD_STRING && (int)mResultRecord->getIndexData()==0)
	{
		ERROR_LOG("��ֵ����DB�����ļ�¼key ����Ϊ0, �����¼������ֹ [%s]", GetEventName());
		mResultType = eRecordKeyError;
		Finish();
		return true;
	}

	mRecordKey = mResultRecord->getIndexData().string();	

	Auto<RequestDBSavePacket> request = mNetConnect->GetNetHandle()->GetNetProtocol()->CreatePacket(PACKET_REQUEST_DBSAVE);
	return request->SendRequestEvent(GetSelf(), mNetConnect.getPtr());
}
//-------------------------------------------------------------------------*/