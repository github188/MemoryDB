
#ifndef _INCLUDE_CSVREADER_H_
#define _INCLUDE_CSVREADER_H_

#include <string>
#ifndef __LINUX__
#   include "Windows.h"
#endif
#include <vector>
#include <map>
#include "FileDataStream.h"
//-----------------------------------------------------------------------------------
struct RowDelta
{
	std::string colName;
	int delta;
	bool isNew;
};

class CsvData 
{	
	struct RowInfo
	{
		int lineNum;
		int levelNum;

		RowInfo() : lineNum(0), levelNum(0)
		{
		}
	};
private:
	std::map<std::string, RowInfo> m_rowPos;
	std::map<std::string, int> m_colPos;
	std::vector<std::vector<std::string> > m_csvData;

public:		
	bool Parser(const char* csvFile);
	//得到具体行列中的数据，level默认为一，但制定level时，如果对应的行列没有数据，则返回空
	//注意，不会对level做安全性检查，如果level数据过大，可能会取到错误的数据
	std::string GetData(const std::string &rolName, const std::string &colName, std::string& type, int rowDelta = 0);
	std::string GetColumNameByIndex(int index);
	void GetRowNames(std::map<std::string,bool>& inMap,bool bCreature);
	int GetColumNum() { return m_colPos.size(); }
	int GetLevelNum(const std::string& rolName);
	int GetDeltaWithPreviousRow(const std::string &rolName, const std::string &colName, bool &isNew);
	std::vector<RowDelta> GetDeltaWithPreviousRow(const std::string &roleName);

	std::string FindRowNameByValue(int nValue, const std::string &colName);
	// 仅为Loading提示
	std::string RandRowName();

public:
	// 为了支持填充NiceTable
	const std::map<std::string, int>& GetFieldName(){ return m_colPos; }
	const std::vector<std::string>& GetData(int row){ return m_csvData[row]; }
	int GetRowNum(){ return m_csvData.size(); }

	bool LoadFromData(const unsigned char* pData, size_t fileSize);

public:
	CsvData();
	virtual ~CsvData();
	CsvData(const CsvData&);
	CsvData& operator=(const CsvData&);
};



//-----------------------------------------------------------------------------------


#endif //_INCLUDE_CSVREADER_H_
