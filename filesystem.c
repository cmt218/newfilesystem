#define _GNU_SOURCE            
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include "support.h"
#include "structs.h"
#include "filesystem.h"


static const int megaByte = 1048576;
static const int FOUR_MB = 4194304;
static const int PAGE_SIZE = 512;


/*
 * generateData() - Converts source from hex digits to
 * binary data. Returns allocated pointer to data
 * of size amt/2.
 */
char* generateData(char *source, size_t size)
{
	char *retval = (char *)malloc((size >> 1) * sizeof(char));

	size_t i;
	for(i=0; i<(size-1); i+=2)
	{
		sscanf(&source[i], "%2hhx", &retval[i>>1]);
	}
	return retval;
}


/*
 * filesystem() - loads in the filesystem and accepts commands
 */
void filesystem(char *file)
{
	/* pointers for the memory-mapped filesystem */
	void *map;
	struct FileSystem* filesystem;
	
	//open/expand file, map memory, handle errors
	int fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (fd == -1){
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    } 
    lseek(fd, FOUR_MB, SEEK_SET);
    write(fd, "", 1);
	map = mmap(NULL, FOUR_MB, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED){
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }

	//initialize the filesystem struct into the mapped memory
	filesystem = map;
	filesystem -> writeTo = map;
	filesystem -> map = map;
 

  	initializeFileSystem(filesystem);
	

	//checkFAT(filesystem);
	
	//printf("%p \n", filesystem -> map);
	//printf("%p \n", filesystem -> writeTo);
	//printf("%ld \n", sizeof(struct FAT));
	//printf("%d \n");
	
	//handle commands
	char *buffer = NULL;
	size_t size = 0;
	while(getline(&buffer, &size, stdin) != -1)
	{
		/* Basic checks and newline removal */
		size_t length = strlen(buffer);
		if(length == 0)
		{
			continue;
		}
		if(buffer[length-1] == '\n')
		{
			buffer[length-1] = '\0';
		}

		/* TODO: Complete this function */
		/* You do not have to use the functions as commented (and probably can not)
		 *	They are notes for you on what you ultimately need to do.
		 */

		if(!strcmp(buffer, "quit"))
		{
			break;
		}
		else if(!strncmp(buffer, "dump ", 5))
		{
			if(isdigit(buffer[5]))
			{
				dump(stdout, atoi(buffer + 5), filesystem);
			}
			else
			{
				//char *filename = buffer + 5;
				char *space = strstr(buffer+5, " ");
				*space = '\0';
				//open and validate filename
				//dumpBinary(file, atoi(space + 1));
			}
		}
		else if(!strncmp(buffer, "usage", 5))
		{
			//usage();
		}
		else if(!strncmp(buffer, "pwd", 3))
		{
			pwd(filesystem);
		}
		else if(!strncmp(buffer, "cd ", 3))
		{
			cd(buffer+3, filesystem);
		}
		else if(!strncmp(buffer, "ls", 2))
		{
			ls(filesystem);
		}
		else if(!strncmp(buffer, "mkdir ", 6))
		{
			myMkdir(buffer+6, filesystem);
		}
		else if(!strncmp(buffer, "cat ", 4))
		{
			//cat(buffer + 4);
		}
		else if(!strncmp(buffer, "write ", 6))
		{
			//char *filename = buffer + 6;
			char *space = strstr(buffer+6, " ");
			*space = '\0';
			size_t amt = atoi(space + 1);
			space = strstr(space+1, " ");

			char *data = generateData(space+1, amt<<1);
			//write(filename, amt, data);
			free(data);
		}
		else if(!strncmp(buffer, "append ", 7))
		{
			//char *filename = buffer + 7;
			char *space = strstr(buffer+7, " ");
			*space = '\0';
			size_t amt = atoi(space + 1);
			space = strstr(space+1, " ");

			char *data = generateData(space+1, amt<<1);
			//append(filename, amt, data);
			free(data);
		}
		else if(!strncmp(buffer, "getpages ", 9))
		{
			//getpages(buffer + 9);
		}
		else if(!strncmp(buffer, "get ", 4))
		{
			//char *filename = buffer + 4;
			char *space = strstr(buffer+4, " ");
			*space = '\0';
			//size_t start = atoi(space + 1);
			space = strstr(space+1, " ");
			//size_t end = atoi(space + 1);
			//get(filename, start, end);
		}
		else if(!strncmp(buffer, "rmdir ", 6))
		{
			//rmdir(buffer + 6);
		}
		else if(!strncmp(buffer, "rm -rf ", 7))
		{
			//rmForce(buffer + 7);
		}
		else if(!strncmp(buffer, "rm ", 3))
		{
			//rm(buffer + 3);
		}
		else if(!strncmp(buffer, "scandisk", 8))
		{
			//scandisk();
		}
		else if(!strncmp(buffer, "undelete ", 9))
		{
			//undelete(buffer + 9);
		}

		free(buffer);
		buffer = NULL;
	}
	free(buffer);
	buffer = NULL;

}

void dump(FILE* file, int fileno, struct FileSystem* filesystem){

	unsigned char* bytes = filesystem->map+512*fileno;
	int i=0;
	while(i<512){
		if(i%16 == 0 && i%32 != 0){
			printf("    ");
		}
		if(i%32 == 0){
			printf("\n");
		}
		printf("%02X ", (unsigned)bytes[i]);
		i++;
	}
	printf("\n");
}

void pwd(struct FileSystem* filesystem){
	
	/* BEGIN STACK IMPLEMENTATION*/
	int maxDepth = 20;
	struct DirectoryPage* path[20];
	int top = -1;
	int isempty(){
		if(top == -1){
			return 1;
		}
		return 0;
	}
	int isfull(){
		if(top == maxDepth){
			return 1;
		}
		return 0;
	}
	struct DirectoryPage* peek(){
		return path[top];
	}
	struct DirectoryPage* pop(){
		struct DirectoryPage* toReturn;
		if(!isempty()){
			toReturn = path[top];
			top --;
			return toReturn;
		}
		return 0;
	}
	void push(struct DirectoryPage* data){
		if(!isfull()){
			top++;
			path[top] = data;
		}
	}
	/*END STACK IMPLEMENTATION*/


	struct DirectoryPage* tmp = filesystem -> currentDirectory;
	while(tmp){
		push(tmp);
		tmp = tmp -> parent;
	}
	tmp = pop();
	while(tmp){
		printf("%s", tmp -> name);
		printf("/");
		tmp = pop();
	}
	printf("\n");
}

void cd(char* dirname, struct FileSystem* filesystem){
	//up a directory case
	if(strcmp(dirname,"..") == 0){
		if(!(strcmp(dirname, "root") == 0)){
			filesystem -> currentDirectory = filesystem -> currentDirectory -> parent;
			return;
		}
	}

	//named directory case
	struct DirectoryPage* tmp = filesystem -> currentDirectory -> childDir;
	while(tmp){
		if(strcmp(dirname, tmp->name) == 0){
			filesystem -> currentDirectory = tmp;
		}
		tmp = tmp -> nextDir;
	}

}

void ls(struct FileSystem* filesystem){
	printf("current directory: %s \n", filesystem -> currentDirectory -> name);
	
	if(filesystem -> currentDirectory -> childDir){
		struct DirectoryPage* tmp = filesystem -> currentDirectory -> childDir;
		while(tmp){
			printf("%s \n", tmp -> name);
			tmp = tmp -> nextDir;
		}
	}
}

/*make a new directory page and place it correctly in the current directory. 
EXPECTS: filesystem -> writeTo to be located on a page boundary*/
void myMkdir(char* dirname, struct FileSystem* filesystem){
	
	//duplicate checking
	struct DirectoryPage* tmp = filesystem -> currentDirectory;
	if(strcmp(tmp -> name, dirname) == 0){
		printf("duplicate name \n");
		return;
	}
	tmp = tmp -> childDir;
	while(tmp){
		if(strcmp(tmp -> name, dirname) == 0){
			printf("duplicate name \n");
			return;
		}	
		tmp = tmp -> nextDir;
	}
	
	//create the new directory page and increment write pointer one block
	struct DirectoryPage* new = filesystem -> writeTo;
	filesystem -> writeTo += PAGE_SIZE;
	new -> parent = filesystem -> currentDirectory;
	strncpy(new -> name, dirname, NAME_MAX);

	//not the first child directory of current directory
	if(filesystem -> currentDirectory -> childDir){
		tmp = filesystem -> currentDirectory -> childDir;
		while(tmp -> nextDir){
			tmp = tmp -> nextDir;
		}
		tmp -> nextDir = new;
	}
	//is the first child directory of current directory
	else{
		filesystem -> currentDirectory -> childDir = new;
	}
}

//currently uses first 377 blocks to initialize the filesystem
void initializeFileSystem(struct FileSystem* filesystem){
	//create a root sector (sector 0) 
	filesystem -> writeTo += sizeof(struct FileSystem);
	struct RootSector* sectorZero = filesystem -> writeTo;
	filesystem -> writeTo += sizeof(struct RootSector);
	filesystem -> rootSec = sectorZero;
	
	//pad write pointer so sector 0 takes a whole block
	filesystem -> writeTo += PAGE_SIZE - (sizeof(struct FileSystem) + sizeof(struct RootSector));
	
	//create allocation table with reference in sector 0
	/*[cmt] FAT is the size of an integral number of pages now but beware of that changing*/
	struct FAT* allocationTable = filesystem -> writeTo;
	sectorZero -> allocationTable = allocationTable;
	filesystem -> writeTo += sizeof(struct FAT);
	
	//create root directory with reference in sector 0 and set filesystem current directory
	struct DirectoryPage* rootDirectory = filesystem -> writeTo;
	sectorZero -> rootDirectory = rootDirectory;
	filesystem -> writeTo += sizeof(struct DirectoryPage);
	strncpy(rootDirectory -> name, "root", NAME_MAX);
	filesystem -> currentDirectory = rootDirectory;
	
	//pad write pointer so root directory page takes a whole block
	filesystem -> writeTo += PAGE_SIZE - (sizeof(struct DirectoryPage));
	

	/******
	 NEED TO FIX THIS
	 *******/
	/* 
	//reflect these structures in FAT
	//[cmt] this loop has to be modified if FAT size ever changes
	FATentry(filesystem, 'r', NULL, 0); // root sector
	struct PageNode* prev = FATentry(filesystem, 'f', NULL, 1); //each FAT block
	int i=0;
	for(i=2;i<=376;i++){
		prev = FATentry(filesystem, 'f', prev, i);
	}
	FATentry(filesystem, 'd', NULL, 377); //root directory
	*/
}

//updates FAT entry for some page and links from previous if necessary. Argument prev is optional
struct PageNode* FATentry(struct FileSystem* filesystem, char type, struct PageNode* prev, unsigned short index){
	//index into allocation table
	struct PageNode* entry = filesystem -> rootSec -> allocationTable -> table;
	entry += (sizeof(struct PageNode)*index);
	
	//update attributes
	entry -> type = type;
	entry -> mapindex = index;
	
	//link from other nodes if necessary
	if(prev){
		prev -> next = entry;
	}
	
	return entry;
}

//prints out whats in the FAT
void checkFAT(struct FileSystem* filesystem){
	
	struct PageNode* table = filesystem -> rootSec -> allocationTable -> table;
	struct PageNode use = *table;

	while(use.type){
		printf("This: %c %d ", use.type, use.mapindex);
		if(use.next){
			printf("Next: %c %d", use.next->type, use.next->mapindex);
		}
		printf("\n");
		table += (sizeof(struct PageNode));
		use = *table;
	}
}

//for testing if things are playing nicely
void blocksUsed(struct FileSystem* filesystem){
	int dif = (filesystem -> writeTo) - (filesystem -> map);
	printf("bytes used: %d \n", dif);
	if(dif%PAGE_SIZE == 0){
		printf("blocks used: %d \n", dif/PAGE_SIZE);
	}
	else{
		printf("writeto not on page boundary");
	}
}


/*
 * help() - Print a help message.
 */
void help(char *progname)
{
	printf("Usage: %s [FILE]...\n", progname);
	printf("Loads FILE as a filesystem. Creates FILE if it does not exist\n");
	exit(0);
}

/*
 * main() - The main routine parses arguments and dispatches to the
 * task-specific code.
 */
int main(int argc, char **argv)
{
	/* for getopt */
	long opt;

	/* run a student name check */
	check_student(argv[0]);

	/* parse the command-line options. For this program, we only support */
	/* the parameterless 'h' option, for getting help on program usage. */
	while((opt = getopt(argc, argv, "h")) != -1)
	{
		switch(opt)
		{
		case 'h':
			help(argv[0]);
			break;
		}
	}

	if(argv[1] == NULL)
	{
		fprintf(stderr, "No filename provided, try -h for help.\n");
		return 1;
	}

	filesystem(argv[1]);
	return 0;
}