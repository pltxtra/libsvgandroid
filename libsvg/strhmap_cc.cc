/* copyright 2014 by Anton Persson */

#include "strhmap_cc.h"
#include <map>
#include <string>

//#define __DO_SVG_DEBUG
#include "svg_debug.h"

StrHmap* StrHmapAlloc(size_t size) {
	return new std::map<std::string, void *>();
}

void  StrHmapClear(StrHmap* hashmap) {
	std::map<std::string, void *> *l = (std::map<std::string, void *> *)hashmap;
	l->clear();
}

int  StrHmapCompact(StrHmap* hashmap) {
	return 0;
}

StrHmap* StrHmapDup(const StrHmap* hashmap) {
	std::map<std::string, void *> *l = (std::map<std::string, void *> *)hashmap;
	std::map<std::string, void *> *neues = new std::map<std::string, void *>();
	*neues = *l;
	return neues; 
}

int  StrHmapErase(StrHmap* hashmap, const char* key) {
	std::map<std::string, void *> *l = (std::map<std::string, void *> *)hashmap;
	std::map<std::string, void *>::iterator k;

	k = l->find(std::string(key));

	if(k == l->end()) return -1;

	l->erase(k);

	return 0;
}

void  StrHmapFree(StrHmap* hashmap) {
	std::map<std::string, void *> *l = (std::map<std::string, void *> *)hashmap;
	delete l;
}

void* StrHmapFind(StrHmap* hashmap, const char* key) {
	std::map<std::string, void *> *l = (std::map<std::string, void *> *)hashmap;
	std::map<std::string, void *>::iterator k;

	k = l->find(std::string(key));
	if(k == l->end()) return NULL;

	return (*k).second;
}

int  StrHmapInsert(StrHmap* hashmap, const char* key, void* item) {
	std::map<std::string, void *> *l = (std::map<std::string, void *> *)hashmap;

	(*l)[std::string(key)] = item;
	return 0;
}

int  StrHmapReplace(StrHmap* hashmap, const char* key, void* item) {
	std::map<std::string, void *> *l = (std::map<std::string, void *> *)hashmap;

	(*l)[std::string(key)] = item;
	return 0;
}

int  StrHmapReserve(StrHmap* hashmap, size_t size) {
	return 0;
}

int StrHmapFindItem(StrHmap* hashmap, void* value) {
	std::map<std::string, void *> *l = (std::map<std::string, void *> *)hashmap;

	for(auto pr : *l) {
		if(pr.second == value) return -1;
	}
	return 0;
}

void StrHmapDump(StrHmap* hashmap) {
	std::map<std::string, void *> *l = (std::map<std::string, void *> *)hashmap;

	SVG_ERROR("StrHmapDump begin:\n");
	for(auto pr : *l) {
		SVG_ERROR("--- %s -> %p\n", pr.first.c_str(), pr.second);
	}
	SVG_ERROR("StrHmapDump end.\n");
}
