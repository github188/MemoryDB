#include "Hand.h"
#include "RecordFieldInfo.h"
#include "BaseTable.h"
#include "IndexBaseTable.h"

AutoRecordFieldInfo::AutoRecordFieldInfo()
{
	setName("RECORD");
}

AutoRecordFieldInfo::~AutoRecordFieldInfo()
{
	if(mOwnerTable)
		mOwnerTable.setNull();
}

bool AutoRecordFieldInfo::saveData(void *data, DataStream *destData) const
{
	AutoRecord temp;
	if(base::get(data, &temp, typeid(AutoRecord)))
	{
		if(temp)
		{
			destData->write(true);
			AutoField fieldInfo = temp->GetTable()->GetField();
			if(!fieldInfo || !fieldInfo->saveToData(destData))
				return false;

			if(!temp->saveData(destData))
				return false;
		}
		else
		{
			destData->write(false);
		}

		return true;
	}
	return false;
}

bool AutoRecordFieldInfo::restoreData(void *data, DataStream *scrData)
{
	bool bHave = false;
	if (scrData->read(bHave))
	{
		if (bHave)
		{
			AutoRecord temp;
			if (!base::get(data, &temp, typeid(AutoRecord)))
				return false;

			AutoField fieldInfo = MEM_NEW BaseFieldIndex();

			if(!fieldInfo->restoreFromData(scrData))
				return false;

			if (!temp)
			{
				if(!mOwnerTable)
					mOwnerTable = tBaseTable::NewFieldTable(false);

				mOwnerTable->ClearAll();
				mOwnerTable->InitField(fieldInfo);

				temp = mOwnerTable->CreateRecord(0, true);

				if (!base::set(data, &temp, typeid(AutoRecord)))
					return false;
			}

			return temp->restoreData(scrData);
		}

		return true;
	}
	return false;
}