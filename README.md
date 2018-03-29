# MemoryDB
一 分布式DB集群

  1 根据哈希槽,实现DB节点分布
  2 使用端同时连接所有的节点


二 基础模块支持 
  1 VS2010 (BaseCommon\Proj2010\BaseCommon.sln)
  2 VS2015 (BaseCommon\proj2015)

  目前未全面支持Linux平台

三 详细分解请关注CSDN
  https://blog.csdn.net/wenge8126/article/details/79729607

四 版权所有,使用请注册原作者

五 示例使用说明
  基于WebSocket 服务应用, 编译环境 VS 2010 
  
  1 安装MySql, 创建home_db和account 两个DB库, 使用utf8字符集

  2 配置DB (ServerRun\RunConfig\MemoryDBConfig.csv)

STRING,STRING,INT,STRING,STRING,STRING,INT,INT,STRING
INDEX,STRING,VALUE,INFO,STRING2,STRING3,VALUE2,VALUE3,STRING4
DataDB,127.0.0.1,3306,,root,root,,,account
DBServer,127.0.0.1,2000,STRING3 如果为LOCAL_DB 表示使用本地文件方式落地;默认为MySql,t_tablelist,MYSQL,,,
DBNode,127.0.0.1,2001,,NO,,,,
MainDBNode,127.0.0.1,2001,,,,,,
*BackDataDB,127.0.0.1,3306,,root,root,,,account_back


DataDB >MySql连接信息
DBServer > 开放给使用端连接的地址,t_tablelist 为DB表的列表信息表格名
DBNode > 当前DB节点的网络地址,端口一般与其他节点连续
MainDBNode > DB主节点地址, 此地址是DB节点启动后,加入到集群的首连接地址


  3 应用端配置 (ServerRun\RunConfig\LoginConfig.csv)
STRING,STRING,INT,INT,INT,STRING,STRING,INT
INDEX,IP,PORT,REGION_ID,SERVER_ID,INFO,DNS,NODE_KEY
LoginIp,127.0.0.1,6002,,1,A服务站,,
NodeNet,127.0.0.1,5000,,,当前节点地址,,-1001
MainGS,127.0.0.1,5000,,,主节点,,-1001
AccountNet,127.0.0.1,2000,,,帐号DB地址,,
GLogData,127.0.0.1,3306,,,Glog日志,,
GateNet,127.0.0.1,4000,,,GS连接网络,,-1001

LoginIp > 开放给客户端连接的WebSocket 地址
NodeNet > 当前进程节点地址
MainGS > 进程节点
AccountNet > account DB 地址(就是2配置的 DBServer)

  4 使用VC2010 打开 Example/Server.sln, 选择X64平台, 选择DebugDevelop, 开始编译

  5 运行 ServerRun/AccountServer_d.exe, ServerRun/LoginServer_d.exe
