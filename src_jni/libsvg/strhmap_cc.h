#include <string.h>

#ifndef _STRHMAP_CC_H
#define _STRHMAP_CC_H

#ifdef __cplusplus
extern "C" {
#endif

	typedef void StrHmap;
	
	StrHmap* StrHmapAlloc(size_t size);
	void  StrHmapClear(StrHmap* hashmap);
	int  StrHmapCompact(StrHmap* hashmap);
	StrHmap* StrHmapDup(const StrHmap* hashmap);
	int  StrHmapErase(StrHmap* hashmap, const char* key);
	void  StrHmapFree(StrHmap* hashmap);
	void* StrHmapFind(StrHmap* hashmap, const char* key);
	int  StrHmapInsert(StrHmap* hashmap, const char* key, void* item);
	int  StrHmapReplace(StrHmap* hashmap, const char* key, void* item);
	int  StrHmapReserve(StrHmap* hashmap, size_t size);
	int StrHmapFindItem(StrHmap* hashmap, void* item);
	void StrHmapDump(StrHmap* hashmap);

#ifdef __cplusplus
}
#endif

#endif
