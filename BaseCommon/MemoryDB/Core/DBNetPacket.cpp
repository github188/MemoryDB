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

	mData->write((int)-1);	// WEB: 需要增加
	mData->write(requstEvt->GetEventID());
	mData->writeString(requstEvt->mTableIndex);
	mData->writeString(requstEvt->mRecordKey);
	mData->writeString(requstEvt->mFunctionName);
	mData->write(requstEvt->mbNeedResponse);	// WEB: 需要增加
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
	
	// NOTE: 转发必须在 mUserConnectID<0 时, 表示未被转发过, 即只准转发一次
	if (!bInThis && mUserConnectID<0 )
	{	
		HandConnect conn = pDBNode->FindNodeByKey(tableIndex.c_str(), recordKey.c_str());
		if (conn)
		{
			// NOTE: 将USER 的连接ID 写入
			mData->seek(0);
			mData->write((int)pConnect->GetNetID());		
			NOTE_LOG("DB 操作发向 NODE [%s:%d] 处理", conn->GetIp(), conn->GetPort());
			conn->Send(this, false);
			return 0;
		}
		NOTE_LOG("ERROR: No exist db [%s] >%s Node", tableIndex.c_str(), recordKey.c_str());
	}

	if (!mData->readString(funName))
		goto error;

	if (!mData->read(mbNeedResponse))
		goto error;

	// 可能未提供，所有只尝试恢复
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
			// NOTE: 如果不需要回复, 当执行成功后, 不再回复到使用端
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
		//	NOTE_LOG("ERROR: RequestDBOperatePacket::Execute 未实现 RequestDBOperatePacket 跨NODE处理 ");
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
	// NOTE: 如果不需要回复, 当执行成功后, 不再回复到使用端
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
		WARN_LOG("[%s] DB处理完成后，连接已释放", bSu ? "成功":"失败");
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
		AssertEx(0, "目前还未设计异步处理，主要是用于多NODE事务");
		NOTE_LOG("目前还未设计异步处理，主要是用于多NODE事务");
		mData->writeString("目前还未设计异步处理，主要是用于多NODE事务");
	}
	else if (op)
	{						
		mData->writeString(op->mErrorInfo);
		if (!op->mResultData->serialize(mData.getPtr(), false))
		{
			ERROR_LOG("ResponseDBOperatePacket::SetResult 操作结果数据序列失败");
		}
	}
	else
		mData->writeString("执行失败");
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

	// NOTE: userConnectID>=0, 表示需要转发, 转发前, 清除此ID, 避免再次被转发失败
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
						// NOTE: 清除转发连接ID, 避免再次被转发失败
						mData->seek(0);
						mData->write(-1);
						conn->Send(this, false);
						return 0;
					}
				}
			}
		}
		NOTE_LOG("ERROR: 原使用端连接未找到，DB操作结果无法送达");
		return 0;
	}

	if (!mData->read(eventID))
		return -1;
	if (!mData->read(result))
		return -1;
	
	mData->readString(resultInfo);

	// 执行错误会，应该不回复执行数据
	mResultData->initData();
	mResultData->restore(mData.getPtr());
	
	// 直接使用等待的事件处理
	ResponseEventFactory *f = dynamic_cast<ResponseEventFactory*>(pConnect->GetNetHandle()->GetEventCenter()->GetResponseEventFactory().getPtr());
	Hand<tServerEvent> hWaitEvt = f->FindServerEvent(eventID); 
	if (hWaitEvt)
	{
		Hand<RequestDBOperateEvent> request = hWaitEvt;
		if (request)
		{		
			if (request->getFinished() )
				request->Log("Warn: 接收到回复时, 已经完成");
			else
			{
				request->OnResponse(result, mResultData, resultInfo);
			}
		}
		// 优化: 直接清除此事件, 有利于事件被重用。 理论上当前还在时间列表中,不清除时，会等待时间超时后再移除处理
		// NOTE: 对于执行操作事件直接释放，内部使用过程中不存在再被使用
		hWaitEvt._free();
	}
	else
	{
		WARN_LOG("回复时, 等待事件不再存在 [%llu]", eventID);
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
			NOTE_LOG("ERROR: RequestDBSavePacket::Execute 未实现 RequestDBSavePacket 跨NODE处理 ");
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
			//ReplyError("字段校验失败 now [%d] of [%d]", checkCode, t->GetField()->GetCheckCode());
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
				ERROR_LOG("ERROR: 未提供正确的记录KEY");
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

	// 直接使用等待的事件处理
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
				request->Log("Warn: 接收到回复时, 已经完成");
			else
			{
				request->OnResponse(result);
			}
		}
		// 优化: 直接清除此事件, 有利于事件被重用。 理论上当前还在时间列表中,不清除时，会等待时间超时后再移除处理
		// NOTE: 对于执行操作事件直接释放，内部使用过程中不存在再被使用
		hWaitEvt._free();
	}
	else
	{
		//WARN_LOG("回复时, 等待事件不再存在 [%llu]", eventID);
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
		mErrorInfo = "未设置需要更新的记录";
		Finish();
		return false;
	}

	if (!mResultRecord->NeedUpdate())
	{
		mResultType = eRecordNotNeedUpdate;
		mErrorInfo = "记录数据不需要更新";
		Finish();
		return false;
	}

	if (!mbGrowthKey && mResultRecord->getField()->getFieldType(0)!=FIELD_STRING && (int)mResultRecord->getIndexData()==0)
	{
		ERROR_LOG("数值类型DB索引的记录key 不可为0, 保存记录数据中止 [%s]", GetEventName());
		mResultType = eRecordKeyError;
		Finish();
		return true;
	}

	mRecordKey = mResultRecord->getIndexData().string();	

	Auto<RequestDBSavePacket> request = mNetConnect->GetNetHandle()->GetNetProtocol()->CreatePacket(PACKET_REQUEST_DBSAVE);
	return request->SendRequestEvent(GetSelf(), mNetConnect.getPtr());
}
//-------------------------------------------------------------------------*/