#ifndef _TEXT_TABLE_H_
#define _TEXT_TABLE_H_

#include "Assertx.h"
#include <string>
//#include <tchar.h>

#include "BaseCommon.h"


typedef   std::string    _tstring;


#ifndef MYASSERT
#define  MYASSERT( a )
#endif


class DataStream;

class BaseCommon_Export CSV
{
public:
	class TableItem
	{
	public:
		TableItem(const char* szItemString);
		bool IsValid() const                { return m_szItemString[0]!=0; }
		operator const       char*() const  { return trim( (char*)m_szItemString ); }
		std::string          String() const { return std::string(trim( (char*)m_szItemString)); }
		const char*          Str() const    { return trim( (char*)m_szItemString); }
		const bool           Bool() const   { return strcmp(m_szItemString, "TRUE") == 0;  }
		const char           Char() const   { return (char)          m_szItemString[0]; }
		const unsigned char  Byte() const   { return (unsigned char) atoi(m_szItemString); }
		const short          Short() const  { return (short)         atoi(m_szItemString); }
		const unsigned short Word() const   { return (unsigned short)atoi(m_szItemString); }
		const int            Int() const    { return                 atoi(m_szItemString); }
		const unsigned int   UInt() const   { return (unsigned int)  atoi(m_szItemString); }
		const long           Long() const   { return                 atol(m_szItemString); }
		const unsigned long  DWord() const  { return (unsigned long) atol(m_szItemString); }
		const float          Float() const  { return (float)         atof(m_szItemString); }
		const double         Double() const { return                 atof(m_szItemString); }

	private:
		const char* m_szItemString;
	};
	class TableLine
	{
	protected:
		TableLine() {}
		void SetLine(CSV* pTable, int iLineIdx)   { m_pTable = pTable; m_iLineIdx = iLineIdx;}
	public:
		friend class CSV;
		const TableItem operator[](int nIndex) const
		{
			return TableItem(m_pTable->GetString(m_iLineIdx, nIndex));
		}
		const TableItem operator[](const char* szIdx) const
		{
			return TableItem(m_pTable->GetString(m_iLineIdx, szIdx));
		}

	private:
		CSV*  m_pTable;
		int                 m_iLineIdx;
	};

	CSV();
	virtual ~CSV();

	// ����һ���ı�����ļ� szFilename
	bool Load(const char *filename);
	bool LoadFromMem( const char* dataBuf,size_t len );

	bool LoadFromData( DataStream  *pDataStream );

	// ���������Ϣ
	void Clear();

	// �Ƿ��Ѿ�����
	bool IsLoaded() { return m_pContent!=NULL; }

	const TableLine& operator[](int nIndex) const   { return m_pLines[nIndex]; }
	const TableLine& operator[](const char* szIdx) const;

	// ȡ��ָ���к��е��ַ���
	const char* GetString(size_t iLine, size_t iCol) const;
	const char* GetString(size_t iLine, const char* szColIdx) const;
	const char* GetString(const char* szLineIdx, const char* szColIdx) const;
	bool Char  (size_t iLine, size_t iCol, char          & vValue) const;
	bool Byte  (size_t iLine, size_t iCol, unsigned char & vValue) const;
	bool Short (size_t iLine, size_t iCol, short         & vValue) const;
	bool Word  (size_t iLine, size_t iCol, unsigned short& vValue) const;
	bool Int   (size_t iLine, size_t iCol, int           & vValue) const;
	bool UInt  (size_t iLine, size_t iCol, unsigned int  & vValue) const;
	bool Long  (size_t iLine, size_t iCol, long          & vValue) const;
	bool DWord (size_t iLine, size_t iCol, unsigned long & vValue) const;
	bool Float (size_t iLine, size_t iCol, float         & vValue) const;
	bool Double(size_t iLine, size_t iCol, double        & vValue) const;

	size_t GetLineCount()           { return m_iLineCount; } // ����ܵ�����
	size_t GetMaxCol()              { return m_iMaxCol; }    // �����������

	void   SetTitleLine(int iIdx)   { m_iTitleLine=iIdx; }   // ����������(0 base)���������ַ����������
	int    GetTitleLine()           { return m_iTitleLine; }
	void   SetTitleCol(int iIdx)    { m_iTitleCol=iIdx; }    // ����������(0 base)���������ַ����������
	int    GetTitleCol()            { return m_iTitleCol; }

	// ���ҹؼ���szString��һ�γ��ֵ�λ�ã��ҵ�����true���ҷ������е�iLine, iCol���򷵻�false
	bool FindPosByString(const char* szString, int& iLine, int& iCol);
	int FindLineByString(const char* szString); // ����szString��һ�γ��ֵ��У��Ҳ�������-1
	int FindColByString(const char* szString);  // ����szString��һ�γ��ֵ��У��Ҳ�������-1

	static char* trim(char* src);

protected:
	bool _LoadData();
	bool ParseTextTable();

	char        m_cSeparator;
	_tstring    m_strFilename;
	char*       m_pContent;
	char**      m_pItems;
	TableLine*  m_pLines;
	TableLine   m_InvalidLine;

	size_t      m_iLineCount;
	size_t      m_iMaxCol;

	int         m_iTitleLine;
	int         m_iTitleCol;

};


#endif
