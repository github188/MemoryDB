#ifndef _INCLUDE_LOOPDATASTREAM_H_
#define _INCLUDE_LOOPDATASTREAM_H_

#include "MemBase.h"
#include "BaseCommon.h"
#include "DataStream.h"
//-------------------------------------------------------------------------*/
//��ʼ���Ľ��ջ��泤��
#define DEFAULTSOCKETINPUTBUFFERSIZE	16*1024
//����������Ļ��泤�ȣ������������ֵ����Ͽ�����
#define DISCONNECTSOCKETINPUTSIZE		128*1024
//-------------------------------------------------------------------------*/
class BaseCommon_Export LoopDataStream : public DataStream
{
public:
	LoopDataStream(UINT BufferSize = DEFAULTSOCKETINPUTBUFFERSIZE, UINT MaxBufferSize = DISCONNECTSOCKETINPUTSIZE);
	virtual ~LoopDataStream( );

	void			SetMaxLength(UINT maxLength){ m_MaxBufferLen = maxLength; }

public:
	virtual DSIZE	tell(void) const{ return 0; }
	virtual bool	seek(DSIZE absolutePosition){ return false; }
	virtual bool	setDataSize(DSIZE dataSize) { return false; }

	virtual bool move( int offSize ) override;

	virtual DSIZE	size(void) const { return m_BufferLen; }
	virtual DSIZE	dataSize(void) const { return Length(); }
	// NOTE: ��ҪС��ʹ�ã��˷���ֻ�ܷ������ݷֲ���һ������
	virtual char*	data(void) override { return m_Buffer + m_Head; }
	virtual bool	resize(DSIZE destSize, char fullValue = 0) { return Resize(destSize)==TRUE; }
	virtual void	clear(bool bFreeBuffer = false){ CleanUp(); }

	virtual bool	_write(void *scrData, DSIZE dataSize) override
	{
		return Write((CHAR*)scrData, dataSize)==dataSize;
	}

	virtual bool	_read(void *destData, DSIZE readSize) override
	{
		return Read((CHAR*)destData, readSize)==readSize;
	}
	inline virtual void* nowData(void){ return NULL; }
	inline virtual DSIZE lastSize(void){ return m_BufferLen-Length()-1; }
	inline virtual DSIZE lastDataSize(void){ return dataSize(); }
	
	inline virtual bool end(void){ return IsEmpty()==TRUE; }

public :
	UINT			Write( const CHAR* buf );

	virtual UINT	Write( const CHAR* buf, UINT len );
	virtual UINT	Read( CHAR* buf, UINT len );
	virtual BOOL	Peek( CHAR* buf, UINT len );
	BOOL			Skip( UINT len );
	VOID			_ForceRestoreHead(UINT formerHead){ m_Head = formerHead; m_Head = m_Head %m_BufferLen; }
	VOID			_ForceRestoreTail(UINT formerTail){ m_Tail = formerTail; m_Tail = m_Tail %m_BufferLen; }

	BOOL			IsEmpty( )const { return m_Head==m_Tail; }
	UINT			Size( )const { return Length(); }
	UINT			Length( )const;
	CHAR*			GetBuffer(){return m_Buffer;}

	UINT			GetHead(){return m_Head;}
	UINT			GetTail(){return m_Tail;}
	UINT			Capacity(){return m_BufferLen;}

	virtual BOOL	Resize( INT size );
	VOID			InitSize(INT size);
	VOID			CleanUp( );

	bool			WriteData( LoopDataStream *scrDataStream );
	bool			WriteData( char *szData, INT len );

	// �õ���һ���ֵ����ݣ����ͷ>β���򷵻�ͷ������������ֵ�����
	CHAR*		GetDataPtr(int &nSize);

	// �õ���һ���ֿ��пռ�
	CHAR*		GetFreeSpacePtr(int &nSize);

protected:
	void			_AllotBuffer( UINT size );
	VOID			InitSize( );

protected :	
	CHAR*		m_Buffer ;

	UINT		m_BufferLen ;
	UINT		m_MaxBufferLen ;

	UINT		m_Head ;
	UINT		m_Tail ;
};

#endif //_INCLUDE_LOOPDATASTREAM_H_