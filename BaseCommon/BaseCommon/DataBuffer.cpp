
//#include "stdafx.h"
#include "DataBuffer.h"

#include "DataStream.h"
#include "MemBase.h"

#include "TableTool.h"

#include "zip/zlib.h"

#include "MemBase.h"

#define LOG_ZIP_TIME		0

#if LOG_ZIP_TIME
#	include "TimeManager.h"
#endif
//--------------------------------------------------------------------------------------

bool Buffer::resize( const DSIZE desLen, char defaultFullValue )
{

	if (desLen==mLength)
		return true;

	if (desLen==0)
	{
		_free();
		return true;
	}

	char *temp = m_pData;

	m_pData = (char*)ALLOCT_NEW(desLen);

	AssertEx(m_pData!=NULL, "Alloct memory fail");

	if (m_pData==NULL)
		return false;

	if (desLen>mLength)
	{
		if (NULL!=temp)
			memcpy(m_pData, temp, mLength);
		memset(m_pData+mLength, defaultFullValue, desLen-mLength);
	}
	else if (NULL!=temp)
		memcpy(m_pData, temp, desLen);

	if (temp!=NULL)
		Allot::freePtr(temp,mLength);

	mLength = desLen;

	return true;
}

bool Buffer::reset( const char* szBuf, const DSIZE length )
{
	if (length==0)
	{
		_free();
		return true;
	}
	
	if (mLength!=length)		
	{
		_free();
		m_pData = (char*)ALLOCT_NEW(length);		
		AssertEx( m_pData, "Now create data is null, create fail" );
		mLength = length;
	}

	memcpy( m_pData, szBuf, length );
	
	return true;
		
}

void Buffer::add( const char *szBuf,const DSIZE lenght )
{

	char *p = (char*)ALLOCT_NEW(lenght);

	if (m_pData)
		memcpy( p,m_pData,mLength );
	memcpy( p+mLength,szBuf,lenght );
	if ( m_pData )
		Allot::freePtr(m_pData,mLength);
	m_pData = p;
	mLength = mLength + lenght;
}

Buffer::Buffer( DSIZE lenght )
{
	m_pData = NULL;
	mLength = 0;
	resize(lenght);
}

Buffer::Buffer( const char* szBuf,const DSIZE lenght )
{
	m_pData = NULL;
	mLength = 0;
	reset( szBuf, lenght );
}
bool Buffer::dele_front( const DSIZE lenght )
{
	if ( lenght == 0 ) return false;
	if ( mLength <= lenght )
	{
		_free();
		return false;
	}
	else
	{
		Buffer	autoBuf( m_pData+lenght, mLength-lenght );
		//reset( autoBuf->m_pData,autoBuf->mLength );
		swap(autoBuf);
	}	
	return true;
}

bool Buffer::dele_back( const DSIZE lenght )
{
	if ( lenght==0 ) return false;
	if ( mLength<= lenght )
	{
		_free();
		return false;
	}
	else
	{
		Buffer autoBuf( m_pData,mLength-lenght );
		//reset( autoBuf->m_pData,autoBuf->mLength );
		swap(autoBuf);
	}
	return true;
}

void Buffer::_free()
{
	if ( m_pData )
	{
		Allot::freePtr(m_pData, mLength);
		m_pData = NULL;
		mLength = 0;
	}
}

bool Buffer::write( DSIZE pos, void* data, DSIZE length )
{
	if ( mLength-pos<length )
		return false;

	memcpy( m_pData+pos, data, length );

	return true;
}

bool Buffer::isOpenState( DSIZE index )
{
	if ( index>=0 && index<mLength*8 )
		return (m_pData[index/8] & ( 1<<(index%8) ))>0;
	return false;
}

bool Buffer::openState( DSIZE index )
{
	if ( index>=0 && index<mLength*8 )
	{
		m_pData[index/8] |= ( 1<<(index%8) );
		return true;
	}
	return false;
}

bool Buffer::closeState( DSIZE index )
{
	if ( index>=0 && index<mLength*8 )
	{
		m_pData[index/8] &= ~( 1<<(index%8) );
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------


DSIZE DataBuffer::ZipData( AutoData &resultBuffer, DSIZE destDataPosition, DSIZE scrDataPos /*= 0*/, DSIZE scrSize /*= 0 */, ZIP_OPTION op )
{
#if LOG_ZIP_TIME
	UINT startTime = TimeManager::NowTick();
#endif

	if (scrSize<=0)
		scrSize = dataSize()-scrDataPos;

	if (scrSize>=0 && data() && size()-scrDataPos>=scrSize )
	{
		unsigned long resize = scrSize + destDataPosition;
		if (!resultBuffer)
			resultBuffer = MEM_NEW DataBuffer(resize);

		unsigned char* szResultData = ((unsigned char*)resultBuffer->data()) + destDataPosition;
		unsigned char* szScrData = ((unsigned char*)data()) + scrDataPos;

		int zOption = Z_BEST_SPEED;
		if (op==STANDARD)
			zOption = Z_DEFAULT_COMPRESSION;
		else if (op = MAX_ZIP_RATE)
			zOption = Z_BEST_COMPRESSION;
		
		while (true)
		{			
			if (resultBuffer->size()<(DSIZE)resize)
			{
				resultBuffer->resize(resize);
				szResultData = ((unsigned char*)resultBuffer->data()) + destDataPosition;
			}
			unsigned long resultZipSize = resize - destDataPosition;

			int re = compress2( szResultData, &resultZipSize, szScrData, (unsigned long)scrSize,  zOption);
			if ( re==Z_OK )
			{
				resultBuffer->seek(destDataPosition+resultZipSize);
				resultBuffer->setDataSize(destDataPosition+resultZipSize);
#if LOG_ZIP_TIME
				TABLE_LOG(" Zip use time > [%u]", TimeManager::NowTick() - startTime);
#endif
				return resultZipSize;
			}
			else if (re==Z_BUF_ERROR)
				resize += 128;
			else
				return 0;
		}
	}
	return 0;
}

bool DataBuffer::UnZipData( AutoData &resultBuffer, DSIZE resultBeginPosition, DSIZE destSize, DSIZE zipDataPos /*= 0*/, DSIZE zipSize /*= 0 */ )
{
#if LOG_ZIP_TIME
	UINT startTime = TimeManager::NowTick();
#endif
	if (zipSize==0)
		zipSize = dataSize()-zipDataPos;
	else if (zipSize > dataSize()-zipDataPos)
		return false;

	if (!resultBuffer)
		resultBuffer = MEM_NEW DataBuffer(resultBeginPosition+destSize);
	else if (resultBuffer->size()<resultBeginPosition+destSize)
		resultBuffer->resize(resultBeginPosition+destSize);

	uLongf destdatasize = (uLongf)destSize;
	if ( uncompress( (unsigned char*)resultBuffer->data()+resultBeginPosition, &destdatasize, (unsigned char*)(data()+zipDataPos), (unsigned long)zipSize )==Z_OK )
	{
		resultBuffer->seek(resultBeginPosition+destdatasize);
		resultBuffer->setDataSize(resultBeginPosition+destdatasize);
#if LOG_ZIP_TIME
		TABLE_LOG(" UNZip use time > [%u]", TimeManager::NowTick() - startTime);
#endif
		return true;
	}

	return false;
}

DSIZE DataBuffer::Zip( void *scrData, DSIZE scrSize, void* resultZipDataBuffer, DSIZE resultSize )
{
	uLongf resultZipSize = (uLongf)resultSize;
	int re = compress2( (unsigned char*)resultZipDataBuffer, &resultZipSize, (unsigned char*)scrData, (unsigned long)scrSize, Z_BEST_SPEED );
	if ( re==Z_OK )
		return resultZipSize;

	return 0;
}

bool DataBuffer::UnZip( void *scrZipData, int scrZipSize, void *resultData, int resultDataSize )
{
	uLongf destdatasize = (uLongf)resultDataSize;
	return ( uncompress( (unsigned char*)resultData, &destdatasize, (unsigned char*)scrZipData, (unsigned long)scrZipSize )==Z_OK );
}

