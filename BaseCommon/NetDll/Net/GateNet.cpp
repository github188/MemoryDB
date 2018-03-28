#include "GateNet.h"
#include "NetIndexPacket.h"

UINT TransferPacket::Execute( tNetConnect* pConnect )
{	
	GateEventProtocol *pPro =dynamic_cast<GateEventProtocol*>(pConnect->GetNetHandle()->GetNetProtocol());
	AssertEx(pPro!=NULL, "Must use GateEventProtocol");

	bool bIsLocal = pPro->CheckInLocal(this, pConnect);
	if (bIsLocal)
	{
		Auto<EventPacket> pDataPacket =  pPro->CreatePacket(mEventPacketID);
		if (pDataPacket)
		{			
			pDataPacket->SetData(mData, mData->dataSize());	
			Logic::tEventCenter *pCenter = pPro->GetTransferEventCenter(pConnect);
			AssertEx(NULL!=pCenter, "�¼����Ĳ�ӦΪ��");
			pDataPacket->GetData()->seek(0);
			AutoEvent hRevEvent = pCenter->RestoreMsg(pDataPacket->GetData().getPtr());

			if (hRevEvent)
			{		
				hRevEvent->setTargetFlag(mTargetFlag);
				hRevEvent->setSendTarget(mTargetID);
				OnDoEventBefore(hRevEvent.getPtr());					
				hRevEvent->_OnBindNet(pConnect);
				pPro->OnNetEventBegin(this, hRevEvent.getPtr());
				pConnect->GetNetHandle()->OnReceiveEvent(pConnect, hRevEvent.getPtr());
			}
			else
			{
				pCenter->Log("�ָ������¼�ʧ��");
				pConnect->GetNetHandle()->GetNetProtocol()->OnPacketExecuteError(pConnect, pDataPacket.getPtr());
				return -1;
			}

			return 0;
		}		
		else
			ERROR_LOG("��ԭ�����¼���Ϣ��ʧ�� > [%d]", mEventPacketID);

		return 0;
	}

	return pPro->Transfer(this, pConnect);

}

HandPacket TransferPacket::CreateSendDataPacket( tNetConnect *pConnect )
{
	EventNetProtocol *pProtocol = dynamic_cast<EventNetProtocol*>(pConnect->GetNetHandle()->GetNetProtocol());
	Auto<EventPacket> pPacket =  pProtocol->CreatePacket(mEventPacketID);
	if (pPacket)
	{
		// ����ת����ֱ���������ݣ������ڽ��ʱ���������Ҫ��ѹ�� SetDataʱ���н�ѹ
		pPacket->_SetSendData(mData, mData->dataSize());	
	}
	else
		ERROR_LOG("Create packet fail>[%d]", mEventPacketID);

	return pPacket.getPtr();
}

Logic::tEventCenter* GateEventProtocol::GetTransferEventCenter(tNetConnect *pConnect)
{
	return pConnect->GetNetHandle()->GetEventCenter();
}

HandPacket GateEventProtocol::GenerateEventPacket(tNetConnect *pConnect,  Logic::tEvent *pSendEvent, bool bNeedZip /*= true*/ )
{
	Auto<EventPacket> pEventPacket = EventNetProtocol::GenerateEventPacket(pConnect, pSendEvent, bNeedZip);
	if (!pEventPacket)
		return pEventPacket;

	int targetFlag = pSendEvent->getTargetFlag();
	if (targetFlag>0)
	{
		Auto<TransferPacket> pTransferPacket = CreatePacket(PACKET_TRANSFER_DATA); //mTransferEventFactory.CreatePacket();
		pTransferPacket->mTargetFlag = (byte)(int)targetFlag;
		pTransferPacket->mTargetID = (int)pSendEvent->getSendTarget();
		pTransferPacket->mEventPacketID = (byte)pEventPacket->GetPacketID();
		// NOTE: ��������Ҫ���Ƕ� pEventPacket �ͷ�, �Ҹ��ư�����, Ŀǰ EventPacket �����йܹ���, ����ֱ��ʹ��
		pTransferPacket->SetData(pEventPacket->GetData(), pEventPacket->GetData()->dataSize());
		return pTransferPacket;
	}
	return pEventPacket;
}

