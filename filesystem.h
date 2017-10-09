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
void cd(char* dirname, struct FileSystem* filesystem);
void pwd(struct FileSystem* filesystem);
void dump(FILE* file, int fileno, struct FileSystem* filesystem);
void usage(struct FileSystem* filesystem);
//void myWrite(struct FileSystem* filesystem, char* filename, size_t amt, char* data);
void cat(struct FileSystem* filesystem, char* filename);
void myWrite(struct FileSystem* filesystem, char* filename, size_t amt, char* data);
struct FileSystem* SequentialFATentry(struct FileSystem* filesystem, char type, struct PageNode* prev,unsigned short end);
void append(struct FileSystem* filesystem, char* filename, size_t amt, char* data);
void rm(struct FileSystem* filesystem, char* filename);
int checkWriteTo(struct FileSystem* filesystem);

//utility prototypes
struct FileSystem* verifyFileSystem(char* file);
struct FileSystem* initializeFileSystem(char* file);
void blocksUsed(struct FileSystem* filesystem);
struct PageNode* FATentry(struct FileSystem* filesystem, char type, struct PageNode* prev, unsigned short index);
void checkFAT(struct FileSystem* filesystem);

#endif
