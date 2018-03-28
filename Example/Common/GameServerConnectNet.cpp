
#include "GameServerConnectNet.h"

#include "GameServerEvent.h"

#include "ServerEvent.h"
//-------------------------------------------------------------------------------------------------------
// ר����GS֮ǰ�Ľ����������⴦��, Ŀ�Ľ�����������ͬ���¼���Ϣ����
// ע��: ��LOGIN �� WORLD��Ҳ����ʹ��
//		��������GS���ڲ������¼���Ϣ, ��Ҫ����������
//-------------------------------------------------------------------------------------------------------

void GSClinetNet::RegisterGSEvent( Logic::tEventCenter *center )
{
	center->RegisterEvent("NodifyClientConnectOk_GR", MEM_NEW EventFactory<SC_NodifyClientConnectOk_C>());
	AString respName = RESP_EVENT_NAME;
	respName += "_GR";
	center->RegisterDefaultEvent( respName.c_str() );

}
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------


void ExecuteEventPacket(tNetHandle *pNet, Packet *pPacket, tConnect *pConnect , Logic::tEventCenter *pEventCenter)
{
	__ENTER_FUNCTION
	AssertEx(pEventCenter!=NULL && pPacket!=NULL && pConnect!=NULL , "�¼�����Ϊ�ջ��Ϊ��, ������Ϊ��");

	EventPacket* p = dynamic_cast<EventPacket*>(pPacket);
	if (p)
	{
		AutoData evtData = p->GetData();
		evtData->seek(0);
		//-------------------------------------------------------------------------
		// �ָ��¼�, ����ʹ�����⴦��, ���Խ��ܵ����¼��������_R, �����ַ����¼�
		AutoEvent hE;
		AString eventName;
		if (evtData->readString(eventName))
		{			
			eventName += "_GR";   //!!! ͳһ��ʽ
			hE = pEventCenter->StartEvent(eventName);
			if (hE)
			{
				if (!hE->_Restore(evtData))
				{
					TableTool::Log("Error: restor event [%s] form data stream", eventName.c_str());
					return;
				}					
			}
			else
			{
				TableTool::Log("Error: restor event form data stream, [%s] no exist", eventName.c_str());
				return;
			}
		}
		else
		{
			TableTool::Log("Error: restor event form data stream");
			return;
		}
		//--------------------------------------------------------------------------
		if (hE)
		{
			hE->_setTarget(pConnect->GetNetID());
			hE->_OnReceive(pNet, pConnect);
			hE->Log("��ʼִ��������Ϣ�¼�...");
			bool re = hE->DoEvent(true);
			hE->Log("ִ�н��[%s]", re ? "TRUE":"FALSE");

			//�ɹ����յ������¼�
			pConnect->OnSucceedReceiveEvent(hE.getPtr());
		}
	}
	else
	{
		TableTool::Log("XXX Error: �����¼����͵���Ϣ��");
	}

	__LEAVE_FUNCTION
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
void GSClinetNet::ExecutePacket( Packet *pPacket, tConnect *pConnect )
{
	ExecuteEventPacket(this, pPacket, pConnect, GetEventCenter());
}


//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
void GSNetHandle::ExecutePacket( Packet *pPacket, tConnect *pConnect )
{
	ExecuteEventPacket(this, pPacket, pConnect, GetEventCenter());
}
//-------------------------------------------------------------------------------------------------------
bool GSNetHandle::OnAddConnect( tConnect *pConnect )
{
	// ���뵽GS��Ϣ��, �Ż���������, �� GS���ʹ��һ������, Ŀǰ, �������Ż�����
	// 1 ���Ӷ�, �ɹ����Ӻ�, �����͵ȴ��ͻ���IP
	// 2 �������Ӷ�, ���յ� IP����ע���, ������������Ϣ, ���뵽GS, 
	// 3 ���ܻ����ظ����, ����ͬʱ�໥����, ��ʱ���ڿ���
	// 4 ���ӱ��Ͽ���, �� OnCloseConnect ��, ����GS��Ϣ

#if !NEED_NET_SAFT_CHECK

	AutoEvent hE = GetEventCenter()->StartEvent("NodifyClientConnectOk");
	hE->set("ID", pConnect->GetNetID());
	Send(hE.getPtr(), pConnect);
	hE->Finish();	
	return true;
#endif
	return false;
}
//-------------------------------------------------------------------------------------------------------

void GSNetHandle::OnCloseConnect( tConnect *pConnect )
{
	// ���� GS ��Ϣ
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------