
//#include "stdafx.h"
//#include <tchar.h>
#ifdef _T
#	define _TC	_T
#else
#	define _TC
#endif

#include "text_table.h"
#include "FileDataStream.h"
#include "TableTool.h"

#ifdef USE_OGRE
	#include "OgreDataStream.h"
	#include "OgreResourceGroupManager.h"
#endif

#ifndef COMMONASSERT
#define COMMONASSERT(a,b)
#endif

CSV::TableItem::TableItem(const char* szItemString)
{
	COMMONASSERT(szItemString,"CSV文件名为空\n" );
	m_szItemString = szItemString;
}

CSV::CSV()
{
	m_cSeparator = ',';
	m_pContent = NULL;
	m_pItems = NULL;
	m_pLines = NULL;

	m_iTitleLine = 0;
	m_iTitleCol = 0;
	m_iMaxCol = 0;
	m_iLineCount = 0;

	m_InvalidLine.SetLine(this, -1);
}

CSV::~CSV()
{
	Clear();
}
//------------------------------------------------------------------


bool CSV::LoadFromMem( const char* dataBuf,size_t len )
{
	if (m_pContent)
	{
		delete[] m_pContent;
	}
	m_pContent = new char[ len+1 ];
	memcpy(m_pContent,dataBuf,len);

	m_pContent[len] = 0;
	return _LoadData();
}

bool CSV::_LoadData()
{
	if (!ParseTextTable())
	{
		Clear();
		return false;
	}

	m_pItems = new char*[m_iLineCount*m_iMaxCol];
	memset(m_pItems, 0, sizeof(char*)*m_iLineCount*m_iMaxCol);

	m_pLines = new TableLine[m_iLineCount];
	for (size_t i=0; i<m_iLineCount; i++)
	{
		m_pLines[i].SetLine(this, int(i));
	}

	ParseTextTable();
	return true;

}

bool CSV::Load(const char *filename)
{
//#pragma warning(push)
//#pragma warning(disable:4996)
//	FILE* fp = _tfopen( filename.c_str(), _TC("rb") );
//#pragma warning(pop)
//	if(fp == NULL)
//	{
////		Log(LOGLEVEL_FATAL, _TC("Cannot Load File: %s"), filename.c_str());
//		return false;
//	}
//
	FileDataStream  fileObj(filename, FILE_READ);

	if (!fileObj)
	{
		ERROR_LOG(("Cannot Load File: %s"), filename);
		return false;
	}

	Clear();

	int iFileSize = fileObj.size();
	//fseek( fp,0,SEEK_END );
	//iFileSize = ftell(fp);
	//fseek( fp,0,SEEK_SET );
	m_strFilename = filename;
	m_pContent = new char[ iFileSize + 1 ];
	m_pContent[iFileSize] = 0;

	if (!fileObj._read(m_pContent, iFileSize))
	{
		ERROR_LOG("read data fail from File: %s", filename);
		return false;
	}
	//fread( m_pContent, iFileSize, 1, fp );
	//fclose(fp);

	if (!ParseTextTable())
	{
		Clear();
		return false;
	}

	m_pItems = new char*[m_iLineCount*m_iMaxCol];
	memset(m_pItems, 0, sizeof(char*)*m_iLineCount*m_iMaxCol);

	m_pLines = new TableLine[m_iLineCount];
	for (size_t i=0; i<m_iLineCount; i++)
	{
		m_pLines[i].SetLine(this, int(i));
	}

	ParseTextTable();
	return true;
}

void CSV::Clear()
{
	m_iTitleLine = 0;
	m_iTitleCol = 0;
	m_iMaxCol = 0;
	m_iLineCount = 0;
	if(m_pContent)
	{
		delete[] m_pContent;
		m_pContent = NULL;
		delete[] m_pItems;
		m_pItems = NULL;
		delete[] m_pLines;
		m_pLines = NULL;
	}
}

bool CSV::ParseTextTable()
{
	AssertEx(m_pContent!=NULL, "read text buffer is null");

	size_t iLine, iCol;
	int iState; // 0 普通, 1 字符串内
	char *pWord, *pCur, *pd;

	iState = 0;
	iLine = iCol = 0;
	pd = pWord = pCur = m_pContent;
	while (*pCur)
	{
		if (iState==0)
		{
			if (*pCur=='"')
			{
				iState = 1;
			}
			else if (*pCur==m_cSeparator)
			{
				if (m_pItems)
				{
					*pd = 0;
					AssertEx(iLine<m_iLineCount && iCol<m_iMaxCol, "Error");
					m_pItems[iLine*m_iMaxCol + iCol] = pWord;
				}
				iCol ++;
				pd = pWord = pCur + 1;
				if (m_pItems==0)
				{
					if (m_iMaxCol<iCol) m_iMaxCol=iCol;
				}
			}
			else if (*pCur==0x0A || *pCur==0x0D)
			{
				if (pCur[1]==0x0A || pCur[1]==0x0D)
				{
					pCur ++;
				}
				if (m_pItems)
				{
					*pd = 0;
					AssertEx(iLine<m_iLineCount && iCol<m_iMaxCol, "Error");
					m_pItems[iLine*m_iMaxCol + iCol] = pWord;
				}
				iCol ++;
				if (m_pItems==0)
				{
					if (m_iMaxCol<iCol) m_iMaxCol=iCol;
				}
				iLine ++;
				iCol = 0;
				pd = pWord = pCur + 1;
			}
			else
			{
				if (m_pItems)
				{
					if (pCur!=pd) *pd = *pCur;
					pd ++;
				}
			}
		}
		else if (iState==1)
		{
			if (*pCur=='"')
			{
				if (pCur[1]=='"')
				{
					// 还是双引号
					pCur ++;
					if (m_pItems)
					{
						if (pCur!=pd) *pd = *pCur;
						pd ++;
					}
				}else
				{
					// 结束
					iState = 0;
				}
			}
			else
			{
				if (m_pItems)
				{
					if (pCur!=pd) *pd = *pCur;
					pd ++;
				}
			}
		}
		pCur ++;
	}
	if (pWord!=pCur)
	{
		if (m_pItems)
		{
			*pd = 0;
			AssertEx(iLine<m_iLineCount && iCol<m_iMaxCol, "Error");
			m_pItems[iLine*m_iMaxCol + iCol] = pWord;
		}
		iCol ++;
		if (m_pItems==0)
		{
			if (m_iMaxCol<iCol) m_iMaxCol=iCol;
		}
		iLine ++;
	}

	m_iLineCount = iLine;
	return true;
}

bool CSV::FindPosByString(const char* szString, int& iLine, int& iCol)
{
	char* p;
	size_t i, j;
	for (i=0; i<m_iLineCount; i++)
	{
		for (j=0; j<m_iMaxCol; j++)
		{
			p = m_pItems[i*m_iMaxCol+j];
			if (p)
			{
				if (strcmp(p, szString)==0)
				{
					iLine = int(i);
					iCol = int(j);
					return true;
				}
			}
		}
	}
	return false;
}

int CSV::FindLineByString(const char* szString)
{
	int iLine, iCol;
	if (FindPosByString(szString, iLine, iCol))
	{
		return iLine;
	}
	return -1;
}

int CSV::FindColByString(const char* szString)
{
	int iLine, iCol;
	if (FindPosByString(szString, iLine, iCol))
	{
		return iCol;
	}
	return -1;
}

const CSV::TableLine& CSV::operator[](const char* szIdx) const
{
	for (size_t i=0; i<m_iLineCount; i++)
	{
		if (strcmp(m_pItems[i*m_iMaxCol+m_iTitleCol], szIdx)==0)
		{
			return m_pLines[i];
		}
	}
	return m_InvalidLine;
}

const char* CSV::GetString(size_t iLine, const char* szColIdx) const
{
	char* pStr;
	static char szNull[] = "";
	if (iLine<0) return szNull;
	for (size_t i=0; i<m_iMaxCol; i++)
	{
		if (strcmp(m_pItems[m_iTitleLine*m_iMaxCol+i], szColIdx)==0)
		{
			pStr = m_pItems[iLine*m_iMaxCol+i];
			if (pStr==NULL) pStr = szNull;
			return pStr;
		}
	}
	return szNull;
}

const char* CSV::GetString(const char* szLineIdx, const char* szColIdx) const
{
	return (*this)[szLineIdx][szColIdx];
}

// 取得指定行和列的字符串
const char* CSV::GetString (size_t iLine, size_t iCol) const
{
	if (iLine<0 || iLine>=m_iLineCount || iCol<0 || iCol>=m_iMaxCol) return NULL;
	if (m_pItems[iLine*m_iMaxCol+iCol]==NULL) return "";
	return (const char*)m_pItems[iLine*m_iMaxCol+iCol];
}

bool CSV::Char (size_t iLine, size_t iCol, char & vValue) const
{
	if (m_pItems[iLine*m_iMaxCol+iCol]==NULL) return false;
	vValue = (char)atoi(m_pItems[iLine*m_iMaxCol+iCol]);
	return true;
}

bool CSV::Byte (size_t iLine, size_t iCol, unsigned char & vValue) const
{
	if (m_pItems[iLine*m_iMaxCol+iCol]==NULL) return false;
	vValue = (unsigned char)atoi(m_pItems[iLine*m_iMaxCol+iCol]);
	return true;
}

bool CSV::Short(size_t iLine, size_t iCol, short & vValue) const
{
	if (m_pItems[iLine*m_iMaxCol+iCol]==NULL) return false;
	vValue = (short)atoi(m_pItems[iLine*m_iMaxCol+iCol]);
	return true;
}

bool CSV::Word (size_t iLine, size_t iCol, unsigned short& vValue) const
{
	if (m_pItems[iLine*m_iMaxCol+iCol]==NULL) return false;
	vValue = (unsigned short)atoi(m_pItems[iLine*m_iMaxCol+iCol]);
	return true;
}

bool CSV::Int  (size_t iLine, size_t iCol, int & vValue) const
{
	if (m_pItems[iLine*m_iMaxCol+iCol]==NULL) return false;
	vValue = (int)atoi(m_pItems[iLine*m_iMaxCol+iCol]);
	return true;
}

bool CSV::UInt (size_t iLine, size_t iCol, unsigned int & vValue) const
{
	if (m_pItems[iLine*m_iMaxCol+iCol]==NULL) return false;
	vValue = (unsigned int)atoi(m_pItems[iLine*m_iMaxCol+iCol]);
	return true;
}

bool CSV::Long (size_t iLine, size_t iCol, long & vValue) const
{
	if (m_pItems[iLine*m_iMaxCol+iCol]==NULL) return false;
	vValue = (long)atol(m_pItems[iLine*m_iMaxCol+iCol]);
	return true;
}

bool CSV::DWord(size_t iLine, size_t iCol, unsigned long & vValue) const
{
	if (m_pItems[iLine*m_iMaxCol+iCol]==NULL) return false;
	vValue = (unsigned long)atol(m_pItems[iLine*m_iMaxCol+iCol]);
	return true;
}

bool CSV::Float (size_t iLine, size_t iCol, float & vValue) const
{
	if (m_pItems[iLine*m_iMaxCol+iCol]==NULL) return false;
	vValue = (float)atof(m_pItems[iLine*m_iMaxCol+iCol]);
	return true;
}

bool CSV::Double(size_t iLine, size_t iCol, double & vValue) const
{
	if (m_pItems[iLine*m_iMaxCol+iCol]==NULL) return false;
	vValue = atof(m_pItems[iLine*m_iMaxCol+iCol]);
	return true;
}

char* CSV::trim(char* src)
{
	if(!src)
		return src;

	while(src)
	{
		if(*src==_TC(' ') || *src==_TC('\t'))
			++src;
		else
			break;
	}
	//int len = (int)strlen( src );
	for ( int i=(int)strlen(src);i>0;i--)
	{
		if ( src[i-1]==' ' )
		{
			src[i-1] = 0;
		}
		else
			break;
	}
	//TCHAR* tail = _tcsstr(src, _TC(" "));
	//if(tail)
	//	*tail=0;

	//tail = _tcsstr(src, _TC("\t"));
	//if(tail)
	//	*tail=0;

	return src;
}

bool CSV::LoadFromData( DataStream *pDataStream )
{
	if (m_pContent)
	{
		delete[] m_pContent;
	
	}
	size_t len = pDataStream->size();
	
	m_pContent = new char[ len+1 ];
	if (!pDataStream->_read(m_pContent, len))
	{
		delete [] m_pContent;
		m_pContent = NULL;
		return false;
	}

	m_pContent[len] = 0;

	return _LoadData();
}
