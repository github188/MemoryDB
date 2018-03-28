

#ifndef __PACKET_H__
#define __PACKET_H__

#include "EventCore.h"
#include "LoopDataStream.h"

//#include "SubPro.h"
#include "MemBase.h"
#include "Auto.h"

//-------------------------------------------------------------------------*/
class Socket;
class Player;

typedef unsigned char	PacketID_t;

#define GET_PACKET_INDEX(a) ((a)>>24)
#define SET_PACKET_INDEX(a,index) ((a)=(((a)&0xffffff)+((index)<<24)))
#define GET_PACKET_LEN(a) ((a)&0xffffff)
#define SET_PACKET_LEN(a,len) ((a)=((a)&0xff000000)+(len))
//��Ϣͷ�а�����PacketID_t-2�ֽڣ�UINT-4�ֽ��и�λһ���ֽ�Ϊ��Ϣ���кţ�����
//�����ֽ�Ϊ��Ϣ����
//ͨ��GET_PACKET_INDEX��GET_PACKET_LEN�꣬����ȡ��UINT�����������Ϣ���кźͳ���
//ͨ��SET_PACKET_INDEX��SET_PACKET_LEN�꣬��������UINT�����������Ϣ���кźͳ���
const int PACKET_HEADER_SIZE = (sizeof(byte)+sizeof(UINT)); //+sizeof(UINT))


//Packet::Execute(...) �ķ���ֵ
enum PACKET_EXE
{
	PACKET_EXE_ERROR = 0 ,
	PACKET_EXE_BREAK ,
	PACKET_EXE_CONTINUE ,
	PACKET_EXE_NOTREMOVE ,
	PACKET_EXE_NOTREMOVE_ERROR ,
	PACKET_EXT_NO_EXESUNPRO,	//û��ע�ắ��
};

class tNetConnect;
class PacketFactory;
class PacketFactoryPtr;
//-------------------------------------------------------------------------*/
class EventCoreDll_Export Packet : public AutoBase //Base<Packet>
{
public :
	BYTE						m_Index;
	Auto<PacketFactoryPtr>	mFactoryPtr;

public :
	Packet( )
		: m_Index(1)
	{}

	virtual ~Packet( ){}

	virtual void InitData() {}

public:
	virtual	PacketID_t	GetPacketID( ) const = 0 ;
	virtual	UINT		GetPacketSize( ) const = 0;
	virtual UINT		GetState() const = 0;
	virtual VOID		SetState(UINT stateData) = 0;

	virtual BOOL		Read( LoopDataStream& iStream, size_t packetSize ) = 0 ;	
	virtual BOOL		Write( LoopDataStream& oStream ) const = 0;
	
	//����ֵΪ��PACKET_EXE �е����ݣ�
	//PACKET_EXE_ERROR ��ʾ�������ش��󣬵�ǰ������Ҫ��ǿ�ƶϿ�
	//PACKET_EXE_BREAK ��ʾ���غ�ʣ�µ���Ϣ�����ڵ�ǰ����ѭ���ﴦ��
	//PACKET_EXE_CONTINUE ��ʾ�����ڵ�ǰѭ����ִ��ʣ�µ���Ϣ
	//PACKET_EXE_NOTREMOVE ��ʾ�����ڵ�ǰѭ����ִ��ʣ�µ���Ϣ,���ǲ����յ�ǰ��Ϣ
	virtual UINT		Execute( tNetConnect* pConnect ) = 0;


	virtual	VOID		SetPacketID( PacketID_t id )  { AssertEx(0, "δʵ������ID"); }
	

	virtual BOOL		CheckPacket( ){ return TRUE ; }

	virtual BYTE		GetPacketIndex( ) const { return m_Index ; }
	virtual VOID		SetPacketIndex( BYTE Index ){ m_Index = Index ; }

	virtual void		SetNeedEncrypt(bool bNeed)const{ }

	virtual VOID		CleanUp( ){}
	virtual void		Release() override;

	Auto<Packet> GetSelf() { return Auto<Packet>(this); }
};
//----------------------------------------------------------------------------------
typedef Auto<Packet>	HandPacket;
//template<>
//void Hand<Packet>::FreeClass(Packet *p)
//{
//	p->Release();
//}
//typedef Hand<Packet>	HandPacket;
//----------------------------------------------------------------------------------

#endif
