#include "FieldSubTable.h"
#include "IndexDBRecord.h"


bool FieldSubTable::AppendRecord(ARecord scrRecord, bool bReplace, bool bRestore)
{
	if (BaseTable::AppendRecord(scrRecord, bReplace))
	{
		// 子表新建记录, 需要全部同步到DB
		IndexDBRecord *p = dynamic_cast<IndexDBRecord*>(scrRecord.getPtr());
		if (p!=NULL)
			p->FullAllUpdate(true);
		mDelectKeyList.erase(scrRecord->getIndexData().string());
		return true;
	}
	return false;
}
