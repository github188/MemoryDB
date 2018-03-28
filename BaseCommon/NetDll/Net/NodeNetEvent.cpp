#include "NodeNetEvent.h"
#include "MeshedNodeNet.h"



MeshedNodeNet* tNodeResponseEvent::GetMeshedNodeNet()
{
	NodeServerNet *pNet = dynamic_cast<NodeServerNet*>(mNetConnect->GetNetHandle());
	AssertEx(pNet!=NULL, "必须为 NodeServerNet");
	return pNet->mOwnerMeshNet;
}

NodeRequestConnect* tNodeRequestEvent::GetNodeConnect()
{
	NodeRequestConnect *p = dynamic_cast<NodeRequestConnect*>( mNetConnect.getPtr() );
	AssertEx (p!=NULL, "必定设定了连接ID");
	return p;
}

MeshedNodeNet* tNodeRequestEvent::GetMeshedNodeNet()
{
	MeshedNodeNet *pNet = dynamic_cast<MeshedNodeNet*>(GetNodeConnect()->GetNetHandle());
	AssertEx(pNet!=NULL, "必须为 MeshedNodeNet");
	return pNet;
}
