#ifndef FILESYSTEM_H
#define FILESYSTEM_H


#include "support.h"
#include "structs.h"

/*
 *	Prototypes for our filesystem functions.
 *
 *
 */

//Help dialog
void help(char *progname);

//Main filesystem loop
void filesystem(char *file);

//Converts source data into appropriate binary data.
//User must free the returned pointer
char* generateData(char *source, size_t size);

//prototypes for commands
void ls(struct FileSystem* filesystem);
void myMkdir(char* dirname, struct FileSystem* filesystem);

//utility prototypes
void initializeFileSystem(struct FileSystem* filesystem);
void blocksUsed(struct FileSystem* filesystem);
struct PageNode* FATentry(struct FileSystem* filesystem, char type, struct PageNode* prev, unsigned short index);
void checkFAT(struct FileSystem* filesystem);

#endif
