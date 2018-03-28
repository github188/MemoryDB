#include "DataProtocol.h"

#include "FileDataStream.h"

bool DataProtocol::LoadProtocol(const char *szTxtFile)
{
    FileDataStream f(szTxtFile, FILE_READ);
    return LoadProtocol(&f);
}

bool DataProtocol::LoadProtocol(DataStream *scrProtocolData, bool bReplace)
{
    AutoTable protocol;
    
    while (true)
    {
        AString lineStr;
        if (!scrProtocolData->readLine(lineStr))
            return true;

        if (lineStr.length()<=0)
            continue;

        if (strstr( lineStr.c_str(), "{")!=NULL)
            return false;

        protocol = tBaseTable::NewBaseTable(false);
        protocol->SetTableName(lineStr.c_str());
        //protocol->AppendField("ID", FIELD_SHORT);
        //protocol->AppendField("TYPE", FIELD_STRING);
        //protocol->AppendField("NAME", FIELD_STRING);
        //protocol->AppendField("VALUE", FIELD_STRING);

        {
            TABLE_LOG("Now load protocol %s", lineStr.c_str());
            bool bHave = false;
            while (true)
            {
                AString line;
                if (! scrProtocolData->readLine(line))
                    return false;

                if (strstr(line.c_str(),"{")!=NULL)
                {
                    bHave = true;
                    break;
                }
                if (line.length()>0)
                    return false;
            }

            if (!bHave)
                return false;

            // 解析数据体
            while (true)
            {
                AString line;
                if (! scrProtocolData->readLine(line) || line.length()<=0)
                    return false;

                if (strstr( line.c_str(), "}")!=NULL)
                    break;

                StringArray strList;
                if (AString::Split(line.c_str(), strList, " ", 5)<3)
                {
                    ERROR_LOG("load protocol %s >%s", protocol->GetTableName(), line.c_str());
                    return false;
                }
                int id = TOINT(strList[0].c_str());
                if (id<=0 || strList[2].length()<=0)
                {
                    ERROR_LOG("load protocol %s >%s", protocol->GetTableName(), line.c_str());
                    return false;
                }
                FieldInfo existField = protocol->GetField()->getFieldInfo(id-1);
                if (existField!=NULL)
                {
                    ERROR_LOG("Already exist %d field <%s>, when load protocol %s, line : %s ", id, existField->getName(), protocol->GetTableName(), line.c_str());
                    return false;
                }
                bool bSubNice = false;
                int type = FIELD_NULL;
                {
                    FieldInfo info = FieldInfoManager::getFieldInfo(strList[1].c_str());
                    if (info==NULL)
                    {
                        if (strList[1].length()>0)
                        {
                            bSubNice = true;
                            type = FIELD_NICEDATA;
                        }
                        else
                        {
                            ERROR_LOG("TYPE error ： load protocol %s >%s", protocol->GetTableName(), line.c_str());
                            return false;
                        }
                    }
                    else
                        type = info->getType();
                }

                AString name = strList[2];
                bool bRepeated = strList.size()>=4 && strList[3] == "+";
                
               if (strstr(name.c_str(), "\t")!=NULL)
                    name.SplitLeft("\t");

                if (name.length()<=0)
                {
                    ERROR_LOG("NAME error ： load protocol %s >%s", protocol->GetTableName(), line.c_str());
                    return false;
                }

                FieldInfo pInfo = protocol->SetField(name.c_str(), (FIELD_TYPE)type, id-1);
                if (pInfo!=NULL)
                {
                    if (bRepeated)
                        pInfo->setAttribute(_REPEATED_ARRAY_FLAG_);
                    if (bSubNice)
                        pInfo->setTypeParam(strList[1]);
                }
                else
                {
                    ERROR_LOG("Set field fail： load protocol %s >%s", protocol->GetTableName(), line.c_str());
                    return false;
                }
            }
            if (protocol->GetField()->check())
            {
                mProtocolList.InsertTable(protocol->GetTableName(), protocol, bReplace);
#if DEVELOP_MODE
                AString test;
                test.Format("TableLog/%s.txt", protocol->GetTableName());
                protocol->SaveCSV(test.c_str(), true);
#endif
            }
            else
            {
                ERROR_LOG("field check fail： load protocol %s", protocol->GetTableName());
                return false;
            }
        }
    }
    return true;
}

bool DataProtocol::SaveProtocol(AutoTable protocol, const char *szFileName, bool bAppend /*= true*/)
{
    FileDataStream f;
    f.open(szFileName, bAppend ? FILE_WRITE : FILE_CREATE);
    if (f)
       return SaveProtocol(protocol, &f);
    return false;
}

bool DataProtocol::SaveProtocol(AutoTable protocol, DataStream *destData)
{
    ARecordIt it = protocol->GetRecordIt();
    destData->_write((void*)protocol->GetTableName(), strlen(protocol->GetTableName()));
    destData->_write("\r\n", 2);
    destData->_write("{\r\n", 3);
    AutoField f = protocol->GetField();
    for (int i=0; i<f->getCount(); ++i)
    {
        FieldInfo info = f->getFieldInfo(i);
        AString strInfo;
        if (info->hasAttribute(_REPEATED_ARRAY_FLAG_))
        {
             if (info->getType()==FIELD_NICEDATA)
             {
                 AString n = info->getName();
                 AString protoName = info->getTypeParam();
                 strInfo.Format("\t%d %s %s +\r\n", i+1, protoName.c_str(), n.c_str());
             }
             else
                strInfo.Format("\t%d %s %s +\r\n", i+1, info->getTypeString(), info->getName().c_str());
        }
        else if (info->getType()==FIELD_NICEDATA)
        {
            AString n = info->getName();
            AString protoName = info->getTypeParam();
             strInfo.Format("\t%d %s %s\r\n", i+1, protoName.c_str(), n.c_str());
        }
        else
        {
                strInfo.Format("\t%d %s %s\r\n", i+1, info->getTypeString(), info->getName().c_str());
        }
        destData->_write((void*)strInfo.c_str(), strInfo.length());
    }
    destData->_write("}\r\n\r\n", 5);
    return true;
}

AutoTable DataProtocol::FindProtocol(const char *szProtocolName)
{
    return mProtocolList.GetTable(szProtocolName);
}

AutoTable DataProtocol::FindProtocol(const AString &strProtocolName)
{
    return mProtocolList.GetTable(strProtocolName.c_str());
}

bool DataProtocol::_SaveNiceData(const char *szProtocolName, NiceData *pScrData, DataStream *destData)
{
    return false;
}

bool DataProtocol::SaveNiceData(AutoTable protocol, NiceData *pScrData, DataStream *pDestData)
{
    AutoField f = protocol->GetField();
    if (!f )
        return false;

    Buffer state((f->getCount()+7)/8);
    if (state.size()>0xFF)
    {
        ERROR_LOG("Save nice data state count over 255");
        return false;
    }

    for (int i=0; i<f->getCount(); ++i)
    {
        FieldInfo info = f->getFieldInfo(i);
        AData &d = pScrData->getAData(info->getName().c_str());
        if ( d.getType()==info->getType()
            || (d.getType()==FIELD_NICEDATA && info->hasAttribute(_REPEATED_ARRAY_FLAG_))
            )
#if DEVELOP_MODE
        {
            state.openState(i);
            info->setAttribute(_SAVE_FLAG_);
        }
        else
            info->removeAttribute(_SAVE_FLAG_);
#else
            state.openState(i);
#endif
    }
#if DEVELOP_MODE
    for (auto it=pScrData->begin(); it->have(); it->next())
    {
        if (it->get().empty())
            continue;
        FieldInfo info = f->getFieldInfo(it->key().c_str());
        if (info==NULL || !info->hasAttribute(_SAVE_FLAG_))
            NOTE_LOG("[%s] save nice data is not save %s", protocol->GetTableName(), info!=NULL ? info->getName().c_str():"NULL");
    }
#endif

    //pDestData->write(f->GetCheckCode());
    pDestData->write((byte)state.size());
    pDestData->_write(state.data(), state.size());

    for (int i=0; i<f->getCount(); ++i)
    {
        if (!state.isOpenState(i))
            continue;
        FieldInfo info = f->getFieldInfo(i);
        AData &niceValue = pScrData->getAData(info->getName().c_str());

         if (info->hasAttribute(_REPEATED_ARRAY_FLAG_))
        {
            AutoNice val2;
            niceValue.get(&val2, typeid(val2));
            Auto<ArrayNiceData> val = val2;
            if (val)
            {
                ArrayNiceData &arrayData = *val;
                Buffer s((arrayData.count()+7)/8);
               for (int n=0; n<val->count(); ++n)
               {
                   AData &d = arrayData.getAData(n);
                   if (!d.empty())
                   {
                       if (d.getType()==info->getType()
                           //|| info->getType()==FIELD_NICEDATA
                           )
                           s.openState(n);
                   }
               }
               pDestData->write((byte)s.size());
               pDestData->_write(s.data(), s.size());
               
               bool bSub = info->getType()==FIELD_NICEDATA;
               AutoTable pro;
               if (bSub)
               {
                   pro= FindProtocol(info->getTypeParam());
                   if (!pro)
                   {
                       ERROR_LOG("No exist sub proto %s", info->getTypeParam().c_str());
                       return false;
                   }
               }
               for (int n=0; n<val->count(); ++n)
               {
                   if (s.isOpenState(n))
                   {
                       AData &d = arrayData.getAData(n);
                       if (bSub)
                       {
                           AutoNice niceValue;
                           d.get(&niceValue, typeid(niceValue));
                           if (niceValue)
                           {
                               pDestData->write(true);
                               if (!SaveNiceData(pro, dynamic_cast<NiceData*>(niceValue.getPtr()), pDestData))
                                   return false;
                           }
                           else
                               pDestData->write(false);
                       }
                       else
                            d._getField()->saveData(d._dataPtr(), pDestData);
                   }
               }
            }
            else
                pDestData->write((byte)0);
        }
         else if (info->getType()==FIELD_NICEDATA)
         {
             AutoNice val;
             niceValue.get(&val, typeid(val));
             if (val)
             {
                 pDestData->write(true);
                 AutoTable pro = FindProtocol(info->getTypeParam());
                 if (!pro)
                 {
                     ERROR_LOG("No exist sub proto %s", info->getTypeParam().c_str());
                     return false;
                 }
                 if (!SaveNiceData(pro, dynamic_cast<NiceData*>(val.getPtr()), pDestData))
                     return false;
             }
             else
                 pDestData->write(false);
             continue;
         }
        else if (!niceValue._getField()->saveData(niceValue._dataPtr(), pDestData) )
        {
            return false;
        }
    }

    return true;
}

bool DataProtocol::_RestoreData(const char *szProtocolName, DataStream *scrData, NiceData *pDestData)
{
    return false;
}

bool DataProtocol::RestoreData(AutoTable protocol, DataStream *scrData, NiceData *pDestData)
{
    AutoField f = protocol->GetField();
    if (!f)
        return false;

    //int checkCode = 0;
    //scrData->read(checkCode);
    //if (checkCode!=f->GetCheckCode())
    //    return false;

    byte stateCount = 0;
    READ(scrData, stateCount);

    Buffer state(stateCount);
    if (!scrData->_read(state.data(), stateCount))
        return false;

    for (int i=0; i<f->getCount(); ++i)
    {
        if (state.isOpenState(i))
        {
            FieldInfo info = f->getFieldInfo(i);
            if (info==NULL)
            {
                ERROR_LOG("No exist field %d", i);
                return false;
            }
            AData &niceValue = pDestData->getOrCreate(info->getName());

            if (info->hasAttribute(_REPEATED_ARRAY_FLAG_))
            {
                byte stateSize=0;
                READ(scrData, stateSize);
                if (stateSize>0)
                {
                    Buffer s(stateSize);
                    scrData->_read(s.data(), stateSize);
                    Auto<ArrayNiceData> val = MEM_NEW ArrayNiceData();
                   ArrayNiceData &arrayData = *val;

                   bool bSub = info->getType()==FIELD_NICEDATA;
                   AutoTable pro;
                   if (bSub)
                   {
                       pro= FindProtocol(info->getTypeParam());
                       if (!pro)
                       {
                           ERROR_LOG("No exist sub proto %s", info->getTypeParam().c_str());
                           return false;
                       }
                   }

                   for (int n=0; n<stateSize*8; ++n)
                   {
                       if (s.isOpenState(n))
                       {
                           AData &d = arrayData.getOrCreate(n);
                           if (bSub)
                           {
                               bool bHave =false;
                               READ(scrData, bHave);
                               if (bHave)
                               {
                                   AutoNice subNice = MEM_NEW NiceData();
                                   if (!RestoreData(pro, scrData, dynamic_cast<NiceData*>(subNice.getPtr())) )
                                       return false;
                                   d.set(&subNice, typeid(subNice));
                               }
                           }
                           else
                           {
                               d.setType(info->getType());
                               if (!d._getField()->restoreData(d._dataPtr(), scrData))
                                   return false;
                           }
                       }
                   }
                   AutoNice val2 = val;
                   niceValue.set(&val, typeid(val2));
                }
            }
            else if (info->getType()==FIELD_NICEDATA)
            {
                bool bHave=false;
                READ(scrData, bHave);
                if (bHave)
                {
                    AutoTable pro = FindProtocol( info->getTypeParam() );
                    if (!pro)
                    {
                        ERROR_LOG("No exist sub proto %s",  info->getTypeParam().c_str());
                        return false;
                    }
                    AutoNice val = MEM_NEW NiceData();
                    if (!RestoreData(pro, scrData, dynamic_cast<NiceData*>(val.getPtr()) ) )
                        return false;
                    niceValue.set(&val, typeid(val));
                }
            }
            else
            {
                niceValue.setType(info->getType());
                if (!niceValue._getField()->restoreData(niceValue._dataPtr(), scrData))
                    return false;
            }
        }
    }


    return true;
}

AutoTable DataProtocol::GenerateProtocol(NiceData *pScrData, const char *szProtocolName, bool bReplace /*= true*/)
{
    AutoTable protocol = tBaseTable::NewBaseTable(false);
    protocol->SetTableName(szProtocolName);

    int nCol = 0;
    for (auto it=pScrData->begin(); it->have(); it->next())
    {
        AData &mainValue = it->get();
        if (!mainValue.empty())
        {
            AString name = it->key();
            int type = mainValue.getType();
            if (type==FIELD_NICEDATA)
            {
                AutoNice val;
                if (mainValue.get(&val, typeid(val)) && val)
                {
                    Auto<NiceData> niceVal = val;
                    if (niceVal)
                    {
                        AString subName;
                        subName.Format("%s_%s", szProtocolName, name.c_str());
                        if (!GenerateProtocol(niceVal.getPtr(), subName.c_str(), true))
                            return AutoTable();
                        FieldInfo info = protocol->SetField(name.c_str(), FIELD_NICEDATA, nCol++);
                        info->setTypeParam(subName);
                    }
                    else
                    {
                        Auto<ArrayNiceData> arrayVal = val;
                        if (arrayVal)
                        {
                            int type = FIELD_NULL;
                            for (int n=0; n<arrayVal->count(); ++n)
                            {
                                AData &d = arrayVal->getAData(n);
                                if (!d.empty())
                                {
                                    type = d.getType();
                                    if (type==FIELD_NICEDATA)
                                    {
                                        AutoNice val;
                                        if (d.get(&val, typeid(val)) && val)
                                        {
                                            Auto<NiceData> niceVal = val;
                                            if (niceVal)
                                            {
                                                AString subName;
                                                subName.Format("%s_%s", szProtocolName, name.c_str());
                                                if (!GenerateProtocol(niceVal.getPtr(), subName.c_str(), true))
                                                    return AutoTable();
                                                FieldInfo info = protocol->SetField(name.c_str(), FIELD_NICEDATA, nCol++);
                                                info->setTypeParam(subName);
                                                info->setAttribute(_REPEATED_ARRAY_FLAG_);
                                                type = FIELD_NULL;
                                                break;
                                            }
                                        }
                                        continue;
                                    }
                                    break;
                                }
                            }
                            if (type!=FIELD_NULL)
                            {
                                protocol->SetField(name.c_str(), (FIELD_TYPE)type, nCol++)->setAttribute(_REPEATED_ARRAY_FLAG_);
                            }
                        }
                    }
                }
            }
            else
            {
                protocol->SetField(name.c_str(), (FIELD_TYPE)type, nCol++);
            }
        }
    }
    if (protocol->GetField()->check())
    {
        mProtocolList.InsertTable(protocol->GetTableName(), protocol, true);
#if DEVELOP_MODE
        AString fileName;
        fileName.Format("TableLog/auto_%s.txt", szProtocolName);
        SaveProtocol(protocol, fileName.c_str(), false);
#endif
    }
    else
        return AutoTable();
    return protocol;
}

bool DataProtocol::SaveAllProtocol(const char *szFileName, bool bAppend /*= false*/)
{
    FileDataStream f;
    f.open(szFileName, bAppend ? FILE_WRITE : FILE_CREATE);
    if (f)
    {
        for (auto it=mProtocolList.GetTableMap().begin(); it; ++it)
        {
            if (!SaveProtocol(it.get(), &f))
                return false;
        }
        return true;
    }
    return false;
}

bool DataProtocol::SerializeProtocal(AutoTable protocal, DataStream *destData)
{
    AutoField f = protocal->GetField();
    if (!f)
    {
        ERROR_LOG("Protocal %s field is NULL", protocal->GetTableName());
        return false;
    }
    destData->writeString(protocal->GetTableName());
    if (!f->saveToData(destData))
    {
        ERROR_LOG("Protocal %s field save fail", protocal->GetTableName());
        return false;
    }
    int count = protocal->GetField()->getCount();
    for (int i=0; i<count; ++i)
    {
        FieldInfo info = f->getFieldInfo(i);
        if (info->getType()==FIELD_NICEDATA)
        {
            AutoTable subProto = FindProtocol(info->getTypeParam());
            if (!subProto)
            {
                ERROR_LOG("No exist sub protocal %s", info->getTypeParam().c_str());
                return false;
            }
            if (!SerializeProtocal(subProto, destData))
            {
                ERROR_LOG("%s Sub protocal %s  save fail", protocal->GetTableName(), subProto->GetTableName());
                return false;
            }
        }
    }
    return true;
}

AutoTable DataProtocol::RestoreProtocal(DataStream *scrData)
{
    AString protocalName = scrData->readString();
    if (protocalName.length()<=0)
    {
        ERROR_LOG("Protocal name restore fail");
        return AutoTable();
    }
    AutoTable protocal = tBaseTable::NewBaseTable(false);
    protocal->SetTableName(protocalName.c_str());
    AutoField f = protocal->GetField();
    if (!f->restoreFromData(scrData))
    {
        ERROR_LOG("%s protocal field restore fail", protocalName.c_str());
        return false;
    }
    int count = f->getCount();
    for (int i=0; i<count; ++i)
    {
        FieldInfo info = f->getFieldInfo(i);
        if (info->getType()==FIELD_NICEDATA)
        {
            AutoTable subPro=RestoreProtocal(scrData);
            if (!subPro)
            {
                ERROR_LOG("Restore sub protocal %s fail", info->getTypeParam().c_str());
                return false;
            }
            if ( info->getTypeParam() != subPro->GetTableName())
            {
                ERROR_LOG("Restore sub protocal %s is not need protocal %s", subPro->GetTableName(), info->getTypeParam().c_str());
                return false;
            }
        }
    }
    AppendProtocol(protocal);
    return protocal;
}
