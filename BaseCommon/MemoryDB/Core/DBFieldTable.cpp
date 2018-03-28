#include "DBFieldTable.h"
#include "TableUpdateLoader.h"
#include "MemoryDBRecord.h"

AutoTable ExDBTableFieldInfo::CreateFieldTable()
{
	if (!mDBTable)
		return AutoTable();
	AutoTable t = MEM_NEW DBFieldTable(this);
	t->InitField(mDBTable->GetField());
	return t;
}

bool ExDBTableFieldInfo::SaveDataToDB(tBaseTable *pScrTable, DataStream *pDBData)
{
    DSIZE pos = pDBData->tell();
    int count = 0;
    pDBData->write(count);
    for (auto it = pScrTable->GetRecordIt(); *it; ++(*it))
    {
        Int64 key = 0;
        if( it->GetKey(key) )
        {
            pDBData->write((int)key);
            ++count;
        }
        else
            ERROR_LOG("[%s] record get key [%s] error", getName().c_str(), it->GetRecord()[0].string().c_str());
    }
    DSIZE lastPos = pDBData->tell();
    pDBData->seek(pos);
    pDBData->write(count);
    pDBData->seek(lastPos);

    return true;
}

bool ExDBTableFieldInfo::RestoreFromDB(DataStream *pScrDBData, tBaseTable *destTable)
{
    if (!mDBTable)
    {
        ERROR_LOG("Sub DB %s no exit", mDBTableName.c_str());
        return false;
    }
    
    pScrDBData->seek(0);
    int count =0; 
    if (!pScrDBData->read(count))
        return false;
    if (count>0)
    {
        if (!destTable)
        {
            destTable = MEM_NEW DBFieldTable(this);
            destTable->InitField(mDBTable->GetField());               
        }
        else
            destTable->ClearAll();
        for (int i=0; i<count; ++i)
        {
            int key;
            if (!pScrDBData->read(key))
                return false;

            AutoRecord r = mDBTable->GetRecord(key);
            if (r)
                destTable->AppendRecord(r, true);
        }
    }
    return true;
}

bool DBFieldTable::AppendRecord(ARecord scrRecord, bool bReplace, bool bRestore /*= false*/)
{
    if (bRestore)
    {
        if (mpDBTableFieldInfo->mDBTable)
        {
            ARecord r = mpDBTableFieldInfo->mDBTable->GetRecord(scrRecord[0].string());                

            if (mpDBTableFieldInfo->mDBTable->AppendRecord(scrRecord, true, false))
            {
                Auto<MemoryDBRecord> newRe = scrRecord;
                // 只要是不存在就设置为新插入
                newRe->SetNewInsert(!r);
                mbChanged = true;
                return NormalBaseTable::AppendRecord(scrRecord, true, false);
            }
        }
        ERROR_LOG("DB field DB table is NULL");
    }
    else
        return NormalBaseTable::AppendRecord(scrRecord, bReplace, false);

    return false;
}
