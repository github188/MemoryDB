#ifndef _INCLUDE_CSVTABLELOADER_H_
#define _INCLUDE_CSVTABLELOADER_H_

#include "BaseTable.h"

//-------------------------------------------------------------------------*/
// CSV 表格读写
// NOTE: 目前运行在Window, 读写经过 ANIS > UTF8 > ANIS 字符集转换
//-------------------------------------------------------------------------*/

class BaseCommon_Export CSVTableLoader : public TableLoader
{
public:
	static bool SaveCSV(tBaseTable *pTable, const char *szCSVFileName, const char *szIndexField = NULL);

public:
	virtual bool SaveTable(tBaseTable *pTable, DataStream *resultData, const char *szIndexField = NULL);

	virtual bool LoadTable(tBaseTable *pTable, DataStream *scrData);

	virtual bool LoadFromData(tBaseTable *pTable, DataStream *scrStream);
};

#define TXT_TABLE_DELIMITED_STRING      "\t"

class BaseCommon_Export TxtTableLoader : public CSVTableLoader
{
public:
    virtual bool SaveTable(tBaseTable *pTable, DataStream *resultData, const char *szIndexField = NULL);
    virtual bool LoadFromData(tBaseTable *pTable, DataStream *scrStream);
};

#endif //_INCLUDE_CSVTABLELOADER_H_