
#include "CSVReader.h"

//-----------------------------------------------------------------------------------
// 使用另外一种读取方法，对UTF8格式及UNCODE的文件支持比较好
//-----------------------------------------------------------------------------------
CsvData::CsvData()
{   
}

CsvData::~CsvData()
{
}

int CsvData::GetLevelNum(const std::string& rolName)
{
	std::map<std::string, RowInfo>::iterator rowIter = m_rowPos.find(rolName);
	if(rowIter == m_rowPos.end())
		return 0;

	return rowIter->second.levelNum;
}

std::string CsvData::GetData(const std::string &rolName, const std::string &colName, std::string& type, int rowDelta)
{
	std::string ret;
	if(colName.empty() || colName.empty())
		return ret;
	std::map<std::string, RowInfo>::iterator rowIter = m_rowPos.find(rolName);
	if(rowIter == m_rowPos.end())
		return ret;
	std::map<std::string, int>::iterator colIter = m_colPos.find(colName);
	if(colIter == m_colPos.end())
		return ret;

	ret = m_csvData[rowIter->second.lineNum + rowDelta][colIter->second];
	type = m_csvData[1][colIter->second];

	return ret;
}

std::vector<RowDelta> CsvData::GetDeltaWithPreviousRow(const std::string &rolName)
{
	std::vector<RowDelta> result;
	for(std::map<std::string, int>::iterator iter = m_colPos.begin(); iter != m_colPos.end(); ++iter)
	{
		RowDelta delta;
		delta.colName = iter->first;
		delta.delta = GetDeltaWithPreviousRow(rolName, delta.colName, delta.isNew);

		if(delta.delta>0)
			result.push_back(delta);
	}
	return result;
}

//make sure data type is int
int CsvData::GetDeltaWithPreviousRow(const std::string &rolName, const std::string &colName, bool& isNew)
{
	isNew = false;
	std::map<std::string, RowInfo>::iterator rowIter = m_rowPos.find(rolName);
	if(rowIter == m_rowPos.end())
		return 0;
	std::map<std::string, int>::iterator colIter = m_colPos.find(colName);
	if(colIter == m_colPos.end())
		return 0;

	int lineNum = rowIter->second.lineNum;
	std::string lData = m_csvData[lineNum][colIter->second];
	if(lData.length() == 0 || lineNum <= 2)
		return 0;

	std::string preData;
	while(lineNum > 2)
	{
		--lineNum;
		if((m_csvData[lineNum][colIter->second]).length() > 0)
		{
			preData = m_csvData[lineNum][colIter->second];
			break;
		}
	}

	int ci = atoi(lData.c_str());
	int pi = 0;
	if(preData.length() > 0)
		pi = atoi(preData.c_str());
	else 
		isNew = true;
	return ci - pi;
}

bool CsvData::Parser(const char* csvFile)
{
	m_csvData.clear();
	m_rowPos.clear();
	m_colPos.clear();

	FileDataStream  fileData(csvFile, FILE_READ);

	AutoData data = fileData.readAllData();

	if (data && data->size()>0)
	{
		return LoadFromData( (const unsigned char*)data->data(), data->size());		
	}

	return false;

}

bool CsvData::LoadFromData(const unsigned char* pData, size_t fileSize)
{
	m_csvData.clear();
	m_rowPos.clear();
	m_colPos.clear();
	if(pData && fileSize > 0)
	{			
		int lastLineNum = 0;
		int lineNum = 0;
		std::string lastRowName;
		unsigned long pos = 0;
		m_csvData.push_back(std::vector<std::string> ());
		for(unsigned long idx = 0; idx < fileSize; ++idx)
		{
			if(pData[idx] == ',' || pData[idx] == '\n' || fileSize - 1 == idx)
			{
				if(pData[pos] == '"' && idx - pos > 1)
					m_csvData[lineNum].push_back(std::string((const char*)(pData + pos + 1), idx- pos - 2)); //去除头尾的 "
				else if(idx > pos)
				{
					if(pData[idx-1] == '\r')
						m_csvData[lineNum].push_back(std::string((const char*)(pData + pos), idx - pos - 1));
					else if(fileSize - 1 == idx)
						m_csvData[lineNum].push_back(std::string((const char*)(pData + pos), idx - pos));
					else
						m_csvData[lineNum].push_back(std::string((const char*)(pData + pos), idx - pos));
				}
				else
					m_csvData[lineNum].push_back(std::string());
				pos = idx + 1;
			}
			if(pData[idx] == '\n' || fileSize - 1 == idx)
			{
				if(fileSize - idx > 2)
				{
					if(lineNum == 0)
					{
						for(size_t colIdx = 0; colIdx < m_csvData[0].size(); ++colIdx)
						{
							m_colPos[m_csvData[0][colIdx]] = colIdx;
						}
					}
					if(lineNum > 1 && !m_csvData[lineNum][0].empty())
					{
						RowInfo rowInfo;
						rowInfo.lineNum = lineNum;
						if(!lastRowName.empty())
							m_rowPos[lastRowName].levelNum = lineNum - lastLineNum;

						lastRowName = m_csvData[lineNum][0];
						lastLineNum = lineNum;
						m_rowPos[lastRowName] = rowInfo;
					}
					++lineNum;
					m_csvData.push_back(std::vector<std::string> ());
				}
				else
				{
					RowInfo rowInfo;											
					if(!m_csvData[lineNum][0].empty())
					{
						rowInfo.lineNum = lineNum;	
						m_rowPos[lastRowName].levelNum = lineNum - lastLineNum;
						lastRowName = m_csvData[lineNum][0];
						rowInfo.levelNum = 1;
						m_rowPos[lastRowName] = rowInfo;
					}
					else
					{
						rowInfo.lineNum = lastLineNum;
						rowInfo.levelNum = lineNum - lastLineNum + 1;
						m_rowPos[lastRowName] = rowInfo;
					}

					break;
				}
			}
		}

		return true;
	}
	else
		return false;
}
std::string CsvData::GetColumNameByIndex(int index)
{
	std::map<std::string, int>::iterator it = m_colPos.begin();
	for (;it != m_colPos.end();++it)
	{
		if (it->second == index)
		{
			return it->first;
		}
	}
	return "";
}

// 仅为奖杯结算
std::string CsvData::FindRowNameByValue(int nValue, const std::string &colName)
{
	std::map<std::string, int>::iterator colIter = m_colPos.find(colName);
	if(colIter == m_colPos.end())
		return "";

	std::string temp = "";
	int nIndex = INT_MAX;
	std::map<std::string, RowInfo>::iterator rowIter = m_rowPos.begin();
	for (;rowIter != m_rowPos.end();++rowIter)
	{
		int nCurIndex = atoi(rowIter->first.c_str());
		int rowNum = rowIter->second.lineNum;
		int colNum = colIter->second;
		std::string strData = m_csvData[rowNum][colNum];
		int nData = atoi(strData.c_str());
		if( nValue <= nData && nIndex >= nCurIndex)
		{
			temp = rowIter->first;
			nIndex = nCurIndex;
		}
	}
	return temp;
}

// 仅为Loading提示
std::string CsvData::RandRowName()
{
	int nMaxIndex = m_rowPos.size();
	//int nMaxIndex = INT_MIN;
	//std::map<std::string, RowInfo>::iterator rowIter = m_rowPos.begin();
	//for (;rowIter != m_rowPos.end();++rowIter)
	//{
	//	int nCurIndex = atoi(rowIter->first.c_str());
	//	if(nCurIndex >= nMaxIndex)
	//		nMaxIndex = nCurIndex;
	//}
	int nRand = rand() % nMaxIndex;
	char temp[32] = {0};
	sprintf(temp, "%d", nRand);
	std::map<std::string, RowInfo>::iterator iter = m_rowPos.find(temp);
	if(iter != m_rowPos.end() )
		return iter->first;

	return "";
}

void CsvData::GetRowNames(std::map<std::string,bool>& inMap,bool bCreature)
{
	std::map<std::string, RowInfo>::iterator it = m_rowPos.begin();
	for (;it != m_rowPos.end();++it)
	{
		if (it->first.compare("Pve") == 0)
		{
			continue;
		}
		inMap.insert(make_pair(it->first,bCreature));
	}
}