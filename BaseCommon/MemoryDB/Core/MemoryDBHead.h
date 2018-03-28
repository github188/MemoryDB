
#ifndef _INCLUDE_MEMORYDBHEAD_H_
#define _INCLUDE_MEMORYDBHEAD_H_
#include "CRC16.h"
// DB ��������
#	define DB_STATE_UPDATE_TIME							(60)				// DB״̬�������ʱ��(��, ���� 60~120)
#	define MEMORY_DB_DEBUG								0
#	define MEMORY_TABLE_PROCESS_SPACE_TIME				10					// �ڴ������������ʱ�䣬���� ���� 0.2~0.5��
#	define RECORD_UPDATE_TIME							(10000)				// ÿ����¼ͬ����DB�ļ��ʱ��, ��ƽ�Ⲣ������, �������6��
#	define DATASCOURCE_OPERATE_ALLOW_COUNT				(100)				// ����Դ�����ȴ������������, ���ڴ�����ʱ, ����Ͷ�����ݲ���, ������ƽ�Ⲣ������, ��Ҫ�������ܱ���

#	define COOL_START_LOAD_RECORD						0					// �䴦�������ʱ�������м�¼
#	define DB_COOL_TIME									(60*60*24*2)		// ��ȴʱ�� �� (���� 2��)
#	define DB_COOL_CHECK_SPACE_TIME						(50*60)				// ���೤ʱ����һ����ȴ
#	define DB_AVAIL_MEMORY_MIN							(16)				// ��С���������ڴ����� (M), С�ڴ�ֵ����ܾ��½���¼ҵ��

#	define DB_HASH_SLOT_COUNT								(16384)			// DB ��Ⱥ��ϣ������
#	define DB_NET_INIT_BUFFER_SIZE							(1024*1024)		// DB ���ӽ����뷢�ͻ���ĳ�ʼ����

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