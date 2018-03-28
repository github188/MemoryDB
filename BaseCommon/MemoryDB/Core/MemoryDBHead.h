
#ifndef _INCLUDE_MEMORYDBHEAD_H_
#define _INCLUDE_MEMORYDBHEAD_H_
#include "CRC16.h"
// DB 性能配置
#	define DB_STATE_UPDATE_TIME							(60)				// DB状态报告更新时间(秒, 建议 60~120)
#	define MEMORY_DB_DEBUG								0
#	define MEMORY_TABLE_PROCESS_SPACE_TIME				10					// 内存表格间隔处理落地时间，毫秒 建议 0.2~0.5秒
#	define RECORD_UPDATE_TIME							(10000)				// 每条记录同步到DB的间隔时间, 可平衡并发能力, 建议大于6秒
#	define DATASCOURCE_OPERATE_ALLOW_COUNT				(100)				// 数据源操作等待最大允许数量, 大于此数量时, 不再投递数据操作, 可智能平衡并发能力, 需要进行性能报告

#	define COOL_START_LOAD_RECORD						0					// 冷处理表启动时加载所有记录
#	define DB_COOL_TIME									(60*60*24*2)		// 冷却时间 秒 (建议 2天)
#	define DB_COOL_CHECK_SPACE_TIME						(50*60)				// 隔多长时间检查一次冷却
#	define DB_AVAIL_MEMORY_MIN							(16)				// 最小正常工作内存限制 (M), 小于此值，会拒绝新建记录业务

#	define DB_HASH_SLOT_COUNT								(16384)			// DB 集群哈希槽数量
#	define DB_NET_INIT_BUFFER_SIZE							(1024*1024)		// DB 连接接收与发送缓存的初始长度

#	define HASH_SLOT(x)			((short)(x % DB_HASH_SLOT_COUNT))
#	define STRHASH_SLOT(sz)		(CRC::crc16((unsigned char*)sz, strlen(sz)) % DB_HASH_SLOT_COUNT)
#	define STRHASH_SLOT(sz)		(CRC::crc16((unsigned char*)sz, strlen(sz)) % DB_HASH_SLOT_COUNT)
#	define ASTR_HASH_SLOT(str) (CRC::crc16((str.c_str(), str.length()) % DB_HASH_SLOT_COUNT)

#endif //_INCLUDE_MEMORYDBHEAD_H_


#ifndef MemoryDB_Export
#ifdef STATE_MEMORYDB_LIB
#	define MemoryDB_Export
#	define MemoryDB_Export_H
#else
#		ifdef MEMORYDB_EXPORTS
#			define MemoryDB_Export		__declspec(dllexport)
#			define MemoryDB_Export_H	__declspec(dllexport)
#		else
#			define MemoryDB_Export		__declspec(dllimport)
#			define MemoryDB_Export_H
#		endif
#endif

#endif