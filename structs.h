#ifndef STRUCTS_H
#define STRUCTS_H
#include <limits.h>
/*
 *
 * Define page/sector structures here as well as utility structures
 * such as directory entries.
 *
 * Sectors/Pages are 512 bytes
 * The filesystem is 4 megabytes in size.
 * You will have 8K pages total.
 *
 */

typedef struct FileSystem{
	void* map;
	void* writeTo;
	struct DirectoryPage* currentDirectory;
	struct RootSector* rootSec;
} Filesystem;

typedef struct RootSector{
	struct FAT* allocationTable;
	struct DirectoryPage* rootDirectory;
} RootSector;

typedef struct DirectoryPage{
	char name[NAME_MAX];
	//[cmt] will also need a pointer to child file eventually (probably a pagenode pointer)
	struct DirectoryPage* parent;
	struct DirectoryPage* childDir;
	struct DirectoryPage* nextDir;
}Directorypage;

typedef struct PageNode{
	char type; //NULL->free, d->directory, r->rootsector, f->FAT, x->raw data
	struct PageNode* next;
	unsigned short mapindex;
} Page;

typedef struct FAT{
	struct PageNode table[8000];
} FAT;


#endif
