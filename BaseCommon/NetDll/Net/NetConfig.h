
#ifndef __INCLUDE_NETCONFIG_H_
#define __INCLUDE_NETCONFIG_H_

//-------------------------------------------------------------------------*/
#define _DEBUG_NET_						1			// 调试网络，如调试信息打印
#define _DEBUG_REMOVE_OVER_TIME			(6)

#define _IOCP_THREAD_RECEIVE_SEND		1			// IOCP网络使用多线接收与发送,  整理测试OK(2018-1-8)
#define _IOCP_THREAD_NUM				(2)

#define _USE_THREAD_SEND_				1			// UDP启动后台线程发送
#define _UDP_SENDTHREAD_ONCE_TIME		(1)			// UDP后台线程每隔10毫秒发送一个包

#define _IOCP_SEND_OVERTIME				(60*5)		// 发送超时时间(秒)，超过此时间报警且关闭连接
#define _ONCE_EXECUTE_MSG_COUNT			(10)		// 一个循环中最多处理的消息个数
const int MAX_SEND_DATA_SIZE			= (1024*1024);	// 最大同时发送数据最大缓存 1M
const int MAX_RECEIVE_DATA_SIZE			= (1024*1024*2);	// 最大同时可接收数据尺寸 2M
#define MAX_PROCESS_PACKET_NUM			(100)		// 一个周期最多允许处理的消息包数量，超过移除
//-----------------------------------------------------------------------------
#define SERVER_NET_CHECK_CONNECT_WAITTIME	60		// 连接安全, 当连接从连接好后, 在这个时间内未接收到数据则进行移除
#define RECEIVE_THREAD_LOOP_SPACETIME		10		// 后台接收线程循环休眠时间, 理论上应与逻辑循环基本一致


#ifdef DEVELOP_MODE
#	define		MIN_NEED_COMPRESS_SIZE		512		//自动处理压缩的最少数据大小, 设定需要压缩, 且数据大小大于或等于此值, 才进行网络消息压缩
// UDP消息数据最大值, 大于此值自动调整为分包发送
// NOTE: UDP 数据包大小为 1472, 包头6个字节, 分包信息12, 理论最佳大小为 1472 - 18, 所有可以考虑1450个字节
const int MAX_UDP_PACKET_SIZE				= 1024;		
const int MAX_UDP_RECEIVE_BUFFER_SIZE		= 2 * 1024;		// 一次接收数据的缓存，一般一个包最大1500， 所以再大也没什么必要
#else
#	define		MIN_NEED_COMPRESS_SIZE		2048		//自动处理压缩的最少数据大小, 设定需要压缩, 且数据大小大于或等于此值, 才进行网络消息压缩

const int MAX_UDP_PACKET_SIZE				= 1280;
const int MAX_UDP_RECEIVE_BUFFER_SIZE		= 2 * 1024;		 
#endif

//-----------------------------------------------------------------------------
#define		UDP_USE_SEND_THREAD						0				// UDP 网络是否使用发送线程发送
#define		UDP_USE_RECEIVE_CHECK_PACKET			0				// UDP 是否启用重发机制处理丢包
#define		EVENTPACKET_DEFAULT_BUFFER_SIZE			(2048)

const int	UDP_DEFAULT_SEND_BUFFER_SIZE			=	(1024*256);	// 默认发送缓存大小
const int	UDP_DEFAULT_RECEIVE_BUFFER_SIZE			=	(1024*1024*1);	// 默认接收缓存 1M

// 后台线程接收方式UDP网络配置
const int	UDP_THREAD_RECEIVE_BUFFER_INIT_SIZE		=	(2 * 1024 * 1024 ); // 后台线程网络初始分配值 2 M, 每个网络对象共需要4M
const int	UDP_THREAD_RECEIVE_BUFFER_MAX_SIZE		=	(16 * 1024 * 1026); // 最大允许分配接收缓存值 16M, 双缓存最多共 32M
const int 	UDP_ONCE_SECOND_SEND_SIZE				=	(10 * 1024);	// 每流限制发送10K
const int	UDP_ONCE_SECOND_SEND_PACKET_COUNT		=	10;				// 每个连接每秒最多发送10个包
const int	UPD_RECORD_SENDED_PACKET_COUNT			=	6;				// 每个连接记录已经发送的包的限定数量
const int	UDP_RECORD_QUEST_LOGIC_EVENTID_COUNT	=	100;			// 每个连接记录最后请求逻辑ID的数据限定数量, 超过后, 会清除最早的ID
//-------------------------------------------------------------------------
#define PACK_COMPART "$-$"//封包分隔符
#define PACK_COMPART_SIZE strlen(PACK_COMPART)


//-------------------------------------------------------------------------
#endif
