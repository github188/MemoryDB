#include "LoopDataStream.h"
#include "TableTool.h"


LoopDataStream::LoopDataStream( UINT BufferSize /*= DEFAULTSOCKETOUTPUTBUFFERSIZE*/, UINT MaxBufferSize /*= DISCONNECTSOCKETOUTPUTSIZE*/ ) 
	: m_Buffer(NULL)
{
	__ENTER_FUNCTION_FOXNET

		m_BufferLen = BufferSize ;
	m_MaxBufferLen = MaxBufferSize ;

	Assert( m_BufferLen > 0 );

	m_Head = 0 ;
	m_Tail = 0 ;

	_AllotBuffer(m_BufferLen);

	__LEAVE_FUNCTION_FOXNET
}

LoopDataStream::~LoopDataStream()
{
	__ENTER_FUNCTION_FOXNET

		ALLOCT_FREE(m_Buffer);

	__LEAVE_FUNCTION_FOXNET
}

UINT LoopDataStream::Write( const CHAR* buf, UINT len )
{
	__ENTER_FUNCTION_FOXNET

		//					//
		//     T  H			//    H   T			LEN=10
		// 0123456789		// 0123456789
		// abcd...efg		// ...abcd...
		//					//

		UINT nFree = ( (m_Head<=m_Tail)?(m_BufferLen-m_Tail+m_Head-1):(m_Head-m_Tail-1) ) ;

	if( len>=nFree )
	{
		if( !Resize( len-nFree+1 ) )
			return 0 ;
	}

	if( m_Head<=m_Tail ) 
	{	
		//if( m_Head==0 ) 
		//{
		//	nFree = m_BufferLen - m_Tail - 1;
		//	memcpy( &m_Buffer[m_Tail], buf, len ) ;
		//} 
		//else 
		{
			nFree = m_BufferLen-m_Tail ;
			if( len<=nFree )
			{
				memcpy( m_Buffer+m_Tail, buf, len ) ;
			}
			else 
			{
				memcpy( m_Buffer+m_Tail, buf, nFree ) ;
				memcpy( m_Buffer, buf+nFree, len-nFree ) ;
			}
		}
	} 
	else 
	{	
		memcpy( &m_Buffer[m_Tail], buf, len ) ;
	}

	m_Tail = (m_Tail+len)%m_BufferLen ;

	return len;

	__LEAVE_FUNCTION_FOXNET

		return 0 ;
}

UINT LoopDataStream::Write( const CHAR* buf )
{
	__ENTER_FUNCTION_FOXNET
		return Write( buf, strlen(buf) ); 
	__LEAVE_FUNCTION_FOXNET
}

UINT LoopDataStream::Read( CHAR* buf, UINT len )
{
	__ENTER_FUNCTION_FOXNET

	if ( buf == NULL || len == 0 )
		return 0 ;

	if ( len > Length() )
		return 0 ;

	if ( m_Head < m_Tail ) 
	{
		memcpy( buf, &m_Buffer[m_Head], len ) ;
	} 
	else 
	{
		UINT rightLen = m_BufferLen-m_Head ;
		if( len<=rightLen ) 
		{
			memcpy( buf, &m_Buffer[m_Head], len ) ;
		} 
		else 
		{
			memcpy( buf, &m_Buffer[m_Head], rightLen ) ;
			memcpy( &buf[rightLen], m_Buffer, len-rightLen ) ;
		}
	}

	m_Head = (m_Head+len)%m_BufferLen ;

	return len ;

	__LEAVE_FUNCTION_FOXNET

		return 0 ;
}

BOOL LoopDataStream::Resize( INT size )
{
	__ENTER_FUNCTION_FOXNET

		Assert( size != 0 ) ;
#ifdef __LINUX__
	size = std::max(size, (int)(m_BufferLen>>1));
#else
	size = MAX(size, (int)(m_BufferLen >> 1));
#endif

	UINT newBufferLen = m_BufferLen + size;
	
	if (newBufferLen>m_MaxBufferLen)
		return FALSE;
	
	UINT len = Length();

	if ( size < 0 ) 
	{
		if ( newBufferLen < 0 || newBufferLen < len )
		{
			AssertEx(0, "新请求分配缓存值不能小于零或都不能小于当前已经存在的数据大小");
			return FALSE ;		
		}
	} 

	CHAR * newBuffer = (CHAR*)ALLOCT_NEW(newBufferLen);
	Assert( newBuffer ) ;

	if ( m_Head < m_Tail ) 
	{
		memcpy( newBuffer , &m_Buffer[m_Head] , m_Tail - m_Head );
	} 
	else if ( m_Head > m_Tail ) 
	{
		memcpy( newBuffer , &m_Buffer[m_Head] , m_BufferLen - m_Head );
		memcpy( &newBuffer[ m_BufferLen - m_Head ] , m_Buffer , m_Tail );
	}

	ALLOCT_FREE( m_Buffer );

	m_Buffer = newBuffer ;
	m_BufferLen = newBufferLen ;
	m_Head = 0 ;
	m_Tail = len ;

	TABLE_LOG("成功重新分配缓存大小 > [%u]", newBufferLen);

	return TRUE ;

	__LEAVE_FUNCTION_FOXNET

		return FALSE ;
}


void LoopDataStream::_AllotBuffer( UINT size )
{
	if (m_Buffer!=NULL)
		ALLOCT_FREE(m_Buffer);
	m_Buffer = (CHAR*)ALLOCT_NEW(size);
	AssertEx(m_Buffer!=NULL, "Allot buffer memory fail");

	memset( m_Buffer, 0, size ) ;
}

BOOL LoopDataStream::Peek( CHAR* buf, UINT len )
{
	__ENTER_FUNCTION_FOXNET

		Assert( buf!=NULL ) ;	

	if( len==0 )
		return FALSE ;

	if( len>Length() )
		return FALSE ;

	if( m_Head<m_Tail ) 
	{
		memcpy( buf , &m_Buffer[m_Head] , len );

	} 
	else 
	{
		UINT rightLen = m_BufferLen-m_Head ;
		if( len<=rightLen ) 
		{
			memcpy( &buf[0], &m_Buffer[m_Head], len ) ;
		} 
		else 
		{
			memcpy( &buf[0], &m_Buffer[m_Head], rightLen ) ;
			memcpy( &buf[rightLen], &m_Buffer[0], len-rightLen ) ;
		}
	}

	return TRUE ;

	__LEAVE_FUNCTION_FOXNET

		return FALSE ;
}

UINT LoopDataStream::Length() const
{
	__ENTER_FUNCTION_FOXNET

		if( m_Head<m_Tail )
			return m_Tail-m_Head;

		else if( m_Head>m_Tail ) 
			return m_BufferLen-m_Head+m_Tail ;

	return 0 ;

	__LEAVE_FUNCTION_FOXNET

		return 0 ;
}


bool LoopDataStream::WriteData( LoopDataStream *scrDataStream )
{
	if (scrDataStream->IsEmpty())
		return true;
	bool b = false;
	if (scrDataStream->m_Head<scrDataStream->m_Tail)
		b = WriteData(scrDataStream->m_Buffer+scrDataStream->m_Head, scrDataStream->m_Tail-scrDataStream->m_Head);
	else
	{
		b = WriteData(scrDataStream->m_Buffer+scrDataStream->m_Head, scrDataStream->m_BufferLen - scrDataStream->m_Head);
		if (b)
			b = WriteData(scrDataStream->m_Buffer, scrDataStream->m_Tail);
	}
	return b;
}

//-------------------------------------------------------------------------
// 向目标空间尝试写数据, 返回是否全部写入
// scrPtr 源数据地址, 执行后偏移到写后的地址
// in_out_scrLen 需要写入长度,返回剩余字节数
bool _CopyData(CHAR* destPtr, int destSize, char **scrPtr, int &in_out_scrLen, UINT &movePos)
{
	if (in_out_scrLen<=destSize)
	{
		memcpy(destPtr, *scrPtr, in_out_scrLen);
		*scrPtr = (*scrPtr)+in_out_scrLen;
		movePos += in_out_scrLen;
		in_out_scrLen = 0;
		return true;
	}
	else
	{
		memcpy(destPtr, *scrPtr, destSize);
		*scrPtr = (*scrPtr) + destSize;
		movePos += destSize;
		in_out_scrLen -= destSize;
		return false;
	}
}
//-------------------------------------------------------------------------

bool LoopDataStream::WriteData( char *szData, INT len )
{
	__ENTER_FUNCTION_FOXNET

		if (szData==NULL || len<=0)
			return 0;

	UINT nReceived = 0 ;
	UINT nFree = 0 ;

	if ( m_Head <= m_Tail ) 
	{
		if ( m_Head == 0 ) 
		{
			//
			// H   T		LEN=10
			// 0123456789
			// abcd......
			//
			nReceived = 0 ;
			nFree = m_BufferLen-m_Tail-1 ;
			if( nFree != 0 )
			{				
				if ( _CopyData(&m_Buffer[m_Tail], nFree, &szData, len, m_Tail))
					return true;			
			}

			if( len > 0 ) 
			{
				UINT available = len;

				if( (m_BufferLen+available+1)>m_MaxBufferLen )
				{
					AssertEx(0, "缓存超过最大分配值");					
					InitSize( );
					return false;
				}
				if( !Resize( available+1 ) )
				{
					AssertEx(0, "分配缓存失败");
					return false;
				}
				if ( _CopyData(&m_Buffer[m_Tail], available, &szData, len, m_Tail) )
					return true;	
				else
				{
					AssertEx(0, "不可能存在不了, 代码错误");
					return false;
				}

			}
		} 
		else 
		{
			//
			//    H   T		LEN=10
			// 0123456789
			// ...abcd...
			//

			nFree = m_BufferLen-m_Tail ;
			nReceived = 0;
			if ( _CopyData(&m_Buffer[m_Tail], nFree, &szData, len, nReceived) )
			{
				m_Tail = (m_Tail+nReceived) % m_BufferLen ;
				return true;	
			}
			m_Tail = (m_Tail+nReceived) % m_BufferLen ;			

			if( nReceived==nFree ) 
			{
				//				Assert( m_Tail == 0 );
				nReceived = 0 ;
				nFree = m_Head-1 ;
				if( nFree!=0 )
				{
					if ( _CopyData(&m_Buffer[0], nFree, &szData, len, nReceived) )
					{
						m_Tail += nReceived;
						return true;
					}
					m_Tail += nReceived;
				}

				if( len>0 ) 
				{
					UINT available = len;

					if( (m_BufferLen+available+1)>m_MaxBufferLen )
					{
						AssertEx(0, "缓存超过最大分配值");
						InitSize( ) ;
						return false;
					}
					if( !Resize( available+1 ) )
					{
						AssertEx(0, "分配缓存失败");
						return false;
					}
					if ( _CopyData(&m_Buffer[m_Tail], available, &szData, len, m_Tail) )
						return true;						
					else
					{
						AssertEx(0, "不可能存在不了, 代码错误");
						return false;
					}

				}
			}
		}

	} 
	else 
	{	
		//
		//     T  H		LEN=10
		// 0123456789
		// abcd...efg
		//

		nReceived = 0 ;
		nFree = m_Head-m_Tail-1 ;
		if( nFree!=0 )
		{
			if ( _CopyData(&m_Buffer[m_Tail], nFree, &szData, len, nReceived) )
			{
				m_Tail += nReceived;
				return true;
			}
			m_Tail += nReceived;
		}
		if( len>0 ) 
		{
			UINT available = len ;
			if ( available>0 ) 
			{
				if( (m_BufferLen+available+1)>m_MaxBufferLen )
				{
					InitSize( ) ;
					AssertEx(0, "缓存超过最大分配值");
					return false;
				}
				if( !Resize( available+1 ) )
				{
					AssertEx(0, "分配缓存失败");
					return false;
				}
				if ( _CopyData(&m_Buffer[m_Tail], available, &szData, len, m_Tail) )
					return true;
				else
				{
					AssertEx(0, "不可能存在不了, 代码错误");
					return false;
				}
			}
		}
	}

	if (len==0)
		return true;
	else
	{
		AssertEx(0, "检查是不是超过最大可分配缓存空间了");
		return false;
	}	

	__LEAVE_FUNCTION_FOXNET

		return false;
}

VOID LoopDataStream::InitSize()
{
	m_Head = 0 ;
	m_Tail = 0 ;

	SAFE_DELETE_ARRAY( m_Buffer ) ;

	_AllotBuffer(DEFAULTSOCKETINPUTBUFFERSIZE);

	m_BufferLen = DEFAULTSOCKETINPUTBUFFERSIZE;
}

VOID LoopDataStream::InitSize(INT size)
{
	if (size>0)
	{
		_AllotBuffer(size);
	}
	m_Head = 0;
	m_Tail = 0;
}

VOID LoopDataStream::CleanUp()
{
	__ENTER_FUNCTION_FOXNET

		m_Head = 0 ;
	m_Tail = 0 ;

	__LEAVE_FUNCTION_FOXNET
}

BOOL LoopDataStream::Skip( UINT len )
{
	__ENTER_FUNCTION_FOXNET

		if( len == 0 )
			return FALSE ;

	if( len>Length( ) )
		return FALSE ;

	m_Head = (m_Head+len)%m_BufferLen ;

	return TRUE ;

	__LEAVE_FUNCTION_FOXNET

		return FALSE ;
}

bool LoopDataStream::move( int offSize )
{
	if (offSize==0)
		return true;
	else if (offSize<0)
		return false;

	return Skip(offSize)==TRUE;
}

CHAR* LoopDataStream::GetDataPtr( int &nSize )
{
	if ( m_Head <= m_Tail )			
		nSize = m_Tail-m_Head;
	else
		nSize = m_BufferLen-m_Head;
	//TableTool::yellow();
	//NOTE_LOG("LoopDataStream > data size %d, head >%u\n", nSize, m_Head);
	return m_Buffer + m_Head;
}

CHAR* LoopDataStream::GetFreeSpacePtr( int &nSize )
{
	if ( m_Head <= m_Tail )			
		nSize = m_BufferLen - m_Tail;
	else
		nSize = m_Head-m_Tail;
	//TableTool::green();
	//NOTE_LOG("LoopDataStream > free space size %d, Tail >%u\n", nSize, m_Tail);
	return m_Buffer + m_Tail;
}
