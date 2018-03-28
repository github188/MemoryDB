
#ifndef _INCLUDE_BIGMSGEVENT_H_
#define _INCLUDE_BIGMSGEVENT_H_

//-------------------------------------------------------------------------
#define WEB_PART_MSG_SEND_SIZE		(30*1024)		// ʵ�ʲ������ؿ���ʹ��16K�����, �ͻ����ϴ����Ϊ2K,�������ֽ������ݲ���������
//-------------------------------------------------------------------------

// ���ʹ������¼�
// NOTE:��ǰͬһ���Ӳ�֧��ͬʱ���Ͷ����������Ϣ
//-------------------------------------------------------------------------
class WEB_SendDataPartEvent : public Logic::tServerEvent
{
protected:
	DataBuffer		mWaitSendData;
	int					mSendCode;
	int					mBigMsgIndexID;

	AutoData			mSendPartData;

public:
	WEB_SendDataPartEvent()
		: mSendCode(0)
		, mBigMsgIndexID(0){}

	virtual void InitData() override
	{
		Logic::tServerEvent::InitData();
		mBigMsgIndexID = 0;
		mSendCode = 0;
		mWaitSendData.clear(false);
	}

public:
	virtual void OnSucceedFinish()
	{ 
		int allSize = (int)mWaitSendData.dataSize();
		int count = (allSize+WEB_PART_MSG_SEND_SIZE-1)/WEB_PART_MSG_SEND_SIZE;
		NOTE_LOG("** Finish send big event data size %d, part count %d", allSize, count);		
	}
	virtual bool _NeedFinishWhenResponsed() const override { return false; }
public:
	bool SendBigEvent(AutoEvent sendEvent)
	{
		mSendCode = 0;
		mWaitSendData.clear(false);
		bool b = GetEventCenter()->SerializeMsg(sendEvent, &mWaitSendData);
		if (b)
		{
			Hand<WebPlayer> player = mNetConnect;
			mBigMsgIndexID = ++player->mLastBigMsgIndexID;

			if (!mSendPartData)
				mSendPartData = MEM_NEW DataBuffer(WEB_PART_MSG_SEND_SIZE);
			int allSize = (int)mWaitSendData.dataSize();
			int count = (allSize+WEB_PART_MSG_SEND_SIZE-1)/WEB_PART_MSG_SEND_SIZE;
			TABLE_LOG("Ready send big event data size %d, part count %d", allSize, count);
			mWaitSendData.seek(0);
			SendOncePart();
			return true;
		}
		return false;
	}

	void SendOncePart()
	{
		int allSize = (int)mWaitSendData.dataSize();
		int count = (allSize+WEB_PART_MSG_SEND_SIZE-1)/WEB_PART_MSG_SEND_SIZE;
		if (mSendCode>=count)
		{
			Finish();
			OnSucceedFinish();
			return;
		}
		setFinished(false);
		GetData().clear(false);
		mSendPartData->clear(false);
		int partSize = mWaitSendData.lastDataSize()>=WEB_PART_MSG_SEND_SIZE ? WEB_PART_MSG_SEND_SIZE : mWaitSendData.lastDataSize();
		mSendPartData->_write(mWaitSendData.data()+mWaitSendData.tell(), partSize);
		mWaitSendData.seek(mWaitSendData.tell()+partSize);
		set("PART", &mSendPartData, typeid(mSendPartData));
		set("ID", mBigMsgIndexID);
		if (mSendCode<=0)
		{
			set("COUNT", count);
			set("SIZE", (int)mWaitSendData.dataSize());
		}
		set("CODE", ++mSendCode);
		WaitTime(10);
		Start();
	}

public:
	virtual void _OnResp(AutoEvent &respEvent) override
	{
		if (respEvent["RESULT"])
		{
			SendOncePart();
		}
		else
			ERROR_LOG("Send part data error, other side receive fail");
	}

	virtual bool _OnTimeOver() override
	{
		TABLE_LOG("Send part data over time");
		return true;
	}
};
//-------------------------------------------------------------------------
class WEB_RevDataPartEvent : public Logic::tClientEvent
{
public:
	virtual bool _DoEvent() override
	{
		Hand<WebPlayer> player = mNetConnect;
		int partCode = get("CODE");
		int id = get("ID");
		AReceiveBigData revData;
		if (partCode==1)
		{
			int count = get("COUNT");
			revData = MEM_NEW WaitReceiveBigMsgData();
			player->mWaitReceiveDataList.insert(id, revData);
			revData->mReceiveBigData.resize(count);
		}
		else
		{
			revData = player->mWaitReceiveDataList.find(id);
			if (!revData)
			{
				ERROR_LOG("�ȴ����մ�������Ϣ ���ݲ����� >%d", id);
				Finish();
				return true;
			}
		}
		Array<AutoData> &waitRevData = revData->mReceiveBigData;
		if (partCode>0 && partCode<=waitRevData.size())
		{
			AutoData d = (DataStream*)get("PART");
			waitRevData[partCode-1] = d;
			GetResponseEvent()["RESULT"] = true;
			if (partCode==waitRevData.size())
			{				
				// ��ɴ���
				static DataBuffer allData(2048);
				allData.clear(false);
				for (int i=0; i<waitRevData.size(); ++i)
				{
					AutoData part = waitRevData[i];
					if (part)
						allData._write(part->data(), part->dataSize());
					else
					{
						ERROR_LOG("����δ���յ������ݰ�");
						GetResponseEvent()["RESULT"] = false;
						Finish();
						return true;
					}
				}

				TABLE_LOG("����������ݰ� %d, total size %d", partCode, (int)allData.dataSize());

				player->ProcessMsg(&allData);
			}
		}
		else
		{
			GetResponseEvent()["RESULT"] = false;
			ERROR_LOG("Part code %d more then size %d", partCode, (int)waitRevData.size());
			player->mWaitReceiveDataList.erase(id);
		}
		Finish();
		return true;
	}
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_BIGMSGEVENT_H_