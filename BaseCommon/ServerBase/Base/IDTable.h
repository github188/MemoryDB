

#ifndef __IDTABLE_H__
#define __IDTABLE_H__
#include "ServerBaseHead.h"
#include "BaseType.h"


struct _TABLEITEM
{
	UINT	m_ID ;
	VOID*	m_pPtr ;
	UINT	m_Status ;
};

class ServerBase_Dll_Export IDTable
{
public :
	IDTable( ) ;
	~IDTable( ) ;

	//��ʼ����
	VOID		InitTable( UINT MaxItem ) ;
	//����һ������
	BOOL		Add( UINT id, VOID* pPtr ) ;
	//��ȡ��Ϣ
	VOID*		Get( UINT id ) ;
	//ɾ������
	VOID		Remove( UINT id ) ;
	//�����������
	VOID		CleanUp( ) ;

protected :
	enum {
		IDTS_EMPTY = 0 ,
		IDTS_SET = 1 ,
		IDTS_USE = 2 ,
	};

	UINT		toIndex( UINT id )
	{
		return (UINT)((id&0xffff)+(id>>6))%m_Count ;
	};

private :

	_TABLEITEM*		m_pTable ;
	UINT			m_Count ;


};

#endif
