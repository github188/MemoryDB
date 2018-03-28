#ifndef _INCLUDE_DATAPROTOCOL_H_
#define _INCLUDE_DATAPROTOCOL_H_

#include "Auto.h"
#include "BaseTable.h"
#include "TableManager.h"
/*/----------------------------------------------------------------------
NiceData 序列优化
    1 协议信息 
    （1）名字
    （2）序号
    （3）类型
    例：
    mm
{
    1 INT age = 10,
        2 STRING name = "NULL",
}
2 字段均为可选
    使用状态数据描述是否存在

    3 协议加载后， 保存为TABLE，使用DataTableManger 保存所有的协议
    并支持保存为协议格式

    4 协议支持嵌套
    解析时， 从协议管理中获取

    5 序列
    （0）4字节整数保存协议验证KEY，通过协议数据生成（当协议准备完成后生成）
    （1）是否存在状态
    （2）使用字段信息进行序列字段
    （3） 如果为嵌套协议，进行子协议序列

    6 反序列
    （1）读取验证， 比较验证，如果不相符报错放弃
    （2）读取存在状态
    （3）根据存在的状态，进行逐个数据读出
    （4）如果为子协议， 则递归反序列子协议
//---------------------------------------------------------------------*/


// 使用 NICEDATA 字段表示为子协议
// NOTE:数组协议时,也是NICEDATA类型, 所有先判断是否为数组
class BaseCommon_Export DataProtocol : public AutoBase
{
public:
    bool LoadProtocol(const char *szTxtFile);
    bool SaveProtocol(AutoTable protocol, const char *szFileName, bool bAppend = true);

    bool LoadProtocol(DataStream *scrProtocolData, bool bReplace = true);

    bool SaveProtocol(AutoTable protocol, DataStream *destData);

    AutoTable FindProtocol(const char *szProtocolName);
    AutoTable FindProtocol(const AString &strProtocolName);

    void AppendProtocol(AutoTable protocal)
    {
        mProtocolList.InsertTable(protocal->GetTableName(), protocal, true);
    }

    bool _SaveNiceData(const char *szProtocolName, NiceData *pScrData, DataStream *destData);

    bool SaveNiceData(AutoTable protocol, NiceData *pScrData, DataStream *destData);

    bool _RestoreData(const char *szProtocolName, DataStream *scrData, NiceData *pDestData);

    bool RestoreData(AutoTable protocol, DataStream *scrData, NiceData *pDestData);

public:
    AutoTable GenerateProtocol(NiceData *pScrData, const char *szProtocolName, bool bReplace = true);

    bool SaveAllProtocol(const char *szFileName, bool bAppend = false);

    bool SerializeProtocal(AutoTable protocal, DataStream *destData);

    AutoTable RestoreProtocal(DataStream *scrData);

public:
    DataTableManager        mProtocolList;
};

#endif //_INCLUDE_DATAPROTOCOL_H_