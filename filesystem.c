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

	struct FileSystem* filesystem;
	
	FILE* test;
	test = fopen(file, "r");
	if(test){
		fclose(test);
		filesystem = verifyFileSystem(file);
	}
	else{
		filesystem = initializeFileSystem(file);
	}


	//blocksUsed(filesystem);	
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
			usage(filesystem);
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
			cat(filesystem, buffer + 4);
		}
		else if(!strncmp(buffer, "write ", 6))
		{
			char *filename = buffer + 6;
			char *space = strstr(buffer+6, " ");
			*space = '\0';
			size_t amt = atoi(space + 1);
			space = strstr(space+1, " ");

			char *data = generateData(space+1, amt<<1);
			myWrite(filesystem, filename, amt, data);
			free(data);
		}
		else if(!strncmp(buffer, "append ", 7))
		{
			char *filename = buffer + 7;
			char *space = strstr(buffer+7, " ");
			*space = '\0';
			size_t amt = atoi(space + 1);
			space = strstr(space+1, " ");

			char *data = generateData(space+1, amt<<1);
			append(filesystem,filename, amt, data);
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
			rm(filesystem,buffer + 3);
		}
		else if(!strncmp(buffer, "scandisk", 8))
		{
			//scandisk();
		}
		else if(!strncmp(buffer, "undelete ", 9))
		{
			//undelete(buffer + 9);
		}
		else if(!strncmp(buffer, "FAT", 3))
		{
			checkFAT(filesystem);
		}
		

		free(buffer);
		buffer = NULL;
	}
	free(buffer);
	buffer = NULL;

}

void getpages(struct FileSystem* filesystem, char* filename){

 struct PageNode* entry = filesystem -> rootSec -> allocationTable -> table;

 if(filesystem->currentDirectory->childDir){

   //printf("%s\n", "we are here");                                                                                                                        

   struct DirectoryPage* temp=filesystem->currentDirectory->childDir;

   while (temp){
   // printf ("%s\n", temp);                                                                                                                                
   //printf("%d\n", strcmp(temp, filename));                                                                                                                

          if(strcmp(temp->name, filename) == 0){
       if(temp->isfile == 1){

     //printf("%s\n", "in while");                                                                                                                          

       entry += temp->index;

     while (entry->next){

       printf("%d, ", entry->mapindex);
 entry=entry->next;
     }

       }
       else{

         printf("%d, ", temp->index);

         if (temp->childDir){

           DirectoryPage* curr = temp->childDir;

           getpages(filesystem, curr->name);

           while(curr->nextDir){
             curr = curr->nextDir;
             getpages(filesystem, curr->name);
           }

         }
         else{
           break;
         }
 }
     }

     if(temp->nextDir){

       temp = temp->nextDir;
     }
     else{
       break;
     }
   }
   if(strcmp(temp->name,filename)!=0){
     printf("File %s not found in directory\n", filename);
   }
 }
}


int checkWriteTo(struct FileSystem* filesystem){
  unsigned char* bytes=filesystem->writeTo;
  int isfilled=0;
  int i=0;
  while(i<512){
    if((unsigned)bytes[i]!=0){
      isfilled=1;
      break;
    }
    i++;

  }
  return isfilled;
}

void rm(struct FileSystem* filesystem, char* filename){
  if(filesystem->currentDirectory->childDir){
    struct DirectoryPage* temp=filesystem->currentDirectory->childDir;
    if(strcmp(temp->name, filename) == 0) {
      int childsize=temp->size;
      
      int childpos=temp->index;
      if(temp->nextDir){
	filesystem->currentDirectory->childDir=temp->nextDir;



      }
      else{
	filesystem->currentDirectory->childDir=NULL;
      }
      temp->parent->size-=temp->size;
      unsigned char* bytes=filesystem->map+512*childpos; 


      filesystem->rootSec->currenttableposition=childpos;
      memset(bytes, 0x00, 512*childsize);
      filesystem->writeTo=&bytes[0];
    }
    else{
      struct DirectoryPage* temp=filesystem->currentDirectory->childDir;
      /*      if(strcmp(temp->name, filename) == 0) {
	int childsize=temp->size;
	int childpos=temp->index;
	unsigned char* bytes=filesystem->map+512*childpos;
	filesystem->rootSec->currenttableposition=childpos;
	memset(bytes, 0x00, 512*childsize);
	filesystem->writeTo=&bytes[0];
      }
      else{*/
      while(temp){
      if(temp->nextDir){
	if(strcmp(temp->nextDir->name, filename) == 0){
	  int nextpos=temp->nextDir->index;
	  int nextsize=temp->nextDir->size;
	  if(temp->nextDir->nextDir){
	    temp->nextDir=temp->nextDir->nextDir;
	  }
	  else{
	    temp->nextDir=NULL;
	  }
	  temp->parent->size-=nextsize;
	  unsigned char* bytes=filesystem->map+512*nextpos;
	  filesystem->rootSec->currenttableposition=nextpos;
	  memset(bytes, 0x00, 512*nextsize);
	  filesystem->writeTo=&bytes[0];

	}
	
	//temp=temp->nextDir;
	  
      }
      else{
      if(strcmp(temp->name, filename) == 0) {
	int childsize=temp->size;

	int childpos=temp->index;
	unsigned char* bytes=filesystem->map+512*childpos;
	temp->parent->size-=temp->size;

	filesystem->rootSec->currenttableposition=childpos;
	memset(bytes, 0x00, 512*childsize);
	filesystem->writeTo=&bytes[0];

      }
      }
      temp=temp->nextDir;
      }
    }
  }
}

void cat(struct FileSystem* filesystem, char* filename){
  
  if(filesystem->currentDirectory->childDir){
    struct DirectoryPage* temp=filesystem->currentDirectory->childDir;
    //if(strncmp(temp->name, filename)!=0){
    while(strcmp(temp->name,filename)!=0){
      if(temp->nextDir)
	temp=temp->nextDir;
      else
	break;
    }
    
    if(!temp->isfile && strcmp(temp->name,filename)==0){
      printf("%s is a directory\n", filename);
    }
    
    else if(strcmp(temp->name,filename)!=0){
      printf("File %s not found in directory\n", filename);
    }
  
    else{
      // dump(stdout, temp->index, filesystem);
      int i=temp->index+1;
      while(i<temp->index+temp->size){
	unsigned char* bytes = filesystem->map+512*(i);
	int j=0;
	int lastbyte=0;
	while(j<512){
	//printf("%02x ", bytes[i]);
	  if((unsigned)bytes[j]!=0){
	    lastbyte=j;
	  }
	  j++; //get last bytes
	}
       
      int a=0;
      for(a=0;a<=lastbyte; a++){
	 printf("%02X ", (unsigned)bytes[a]);
	//printf("%d %c ", (char)bytes[j]);
      }
      i=i+1;
      }
  
    }
  }
}

void myWrite(struct FileSystem* filesystem, char* filename, size_t amt, char* data){
 
 	while(checkWriteTo(filesystem)) {
    filesystem->writeTo+=512;
    filesystem -> rootSec -> currenttableposition+=1;
    //filesystem=SequentialFATentry(filesystem, NULL, NULL, 1);
    }
 
    struct DirectoryPage* newchild=(struct DirectoryPage*)filesystem->writeTo; 
 
    unsigned short filestartpos=filesystem->rootSec->currenttableposition;
    filesystem->writeTo+=PAGE_SIZE;
    filesystem=SequentialFATentry(filesystem, 'f', NULL, 1);
                                    
									      
    
    newchild->isfile=1;
    newchild->index=filestartpos;
    newchild->parent=filesystem->currentDirectory;
    strncpy(newchild->name,filename, NAME_MAX);
    
    // filesystem=FATentry(filesystem, 'f', NULL, (unsigned short)((amt+(512-(amt%512)))/512));
    newchild->nextDir=NULL;
    newchild->childDir=NULL;
    if(filesystem->currentDirectory->childDir){
      struct DirectoryPage* temp=filesystem->currentDirectory->childDir;
      while(temp->nextDir){
	temp=temp->nextDir;
      }
      temp->nextDir=newchild;
    }
    else{
      filesystem->currentDirectory->childDir=newchild;
    }
    memcpy(filesystem->writeTo, data, amt);
    
    if(amt%PAGE_SIZE==0){
      filesystem->writeTo+=amt;
      filesystem=SequentialFATentry(filesystem, 'f', NULL, (unsigned short)(amt/512));
    }

    else{
      filesystem->writeTo+=amt+(PAGE_SIZE-(amt%PAGE_SIZE)); //update
							    //writeTo
      filesystem=SequentialFATentry(filesystem, 'f', NULL, (unsigned short)((amt+(512-(amt%512)))/512));
    }

    int noPages = 0;
    noPages = 1 + (amt/512);

    if(amt%512 > 0){
    	noPages++;
    }

    newchild->size = noPages;
    newchild->parent->size += noPages;
    newchild->currentposition = amt;

}

void append(struct FileSystem* filesystem, char* filename, size_t amt, char* data){
  
  if(filesystem->currentDirectory->childDir){
    struct DirectoryPage* temp=filesystem->currentDirectory->childDir;
    //if(strncmp(temp->name, filename)!=0){                                                              
    while(strcmp(temp->name,filename)!=0){
      if(temp->nextDir){
        temp=temp->nextDir;
      }
      else{
	break;
      }
    }
  
    if(!temp->isfile && strcmp(temp->name,filename)==0){
      printf("%s is a directory\n", filename);
    }

    else if(strcmp(temp->name,filename)!=0){
      printf("File %s not found in directory\n", filename);
    }
	
    else{
	unsigned char* currentpos=(filesystem->map+(512*(temp->index+1))+temp->currentposition);

        memcpy(currentpos, data, amt);
		//currentposition=currentposition+amt;
	temp->currentposition=temp->currentposition+amt;
    }
  }
}
void usage(struct FileSystem* filesystem){
	printf("bytes used: %d \n", filesystem->blocksUsed*512);
	printf("bytes used by files: %d \n", filesystem->fileCt*512);
}


void dump(FILE* file, int fileno, struct FileSystem* filesystem){

	unsigned char* bytes = (unsigned char*)filesystem->map+512*fileno;
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
		tmp = pop();
		if(tmp){
			printf("/");
		}
	}
	printf("\n");
}

void cd(char* dirname, struct FileSystem* filesystem){
	//up a directory case
	if(strcmp(dirname,"..") == 0 ){
		if(!(strcmp(filesystem->currentDirectory->name, "root") == 0)){
			filesystem -> currentDirectory = filesystem -> currentDirectory -> parent;
			return;
		}
	}

	//named directory case
	struct DirectoryPage* tmp = filesystem -> currentDirectory -> childDir;
	while(tmp){

		if(strcmp(dirname, tmp->name) == 0){
			if(tmp->isfile){
				printf("-bash cd: %s: Not a directory\n", tmp->name);
			}
			else{
			filesystem -> currentDirectory = tmp;
			}
		}
		tmp = tmp -> nextDir;
	}

}

void ls(struct FileSystem* filesystem){
	if(filesystem -> currentDirectory -> childDir){
		struct DirectoryPage* tmp = filesystem -> currentDirectory -> childDir;
		while(tmp){
			if(tmp->isfile){
				printf("f ");
			}
			else{
				printf("d ");
			}
			printf("%d ", tmp->size*512);
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
	struct DirectoryPage* new = (struct DirectoryPage*)filesystem -> writeTo;
	new -> size = 1;
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
	
	filesystem->currentDirectory->childDir->index = filesystem->rootSec->currenttableposition;

	//increment counts and make FAT entry
	filesystem -> blocksUsed++;
	FATentry(filesystem, 'd', NULL, filesystem -> rootSec -> currenttableposition);
	filesystem -> rootSec -> currenttableposition++;

}

struct FileSystem* verifyFileSystem(char* file){
	struct FileSystem* filesystem;

	printf("verifying \n");
	
	char * map;

	int fd = open(file, O_RDWR, 0777);
	if (fd == -1){
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    //lseek(fd, FOUR_MB, SEEK_SET);
    //write(fd, "", 1);
    //map = mmap(NULL, FOUR_MB, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	map = mmap((void*) 0x7ffff7be0000, FOUR_MB, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0);

	if (map == MAP_FAILED){
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }
	
	filesystem = (struct FileSystem*)map;
	//filesystem -> map = map;

  	//*filesystem = *(struct FileSystem*)map;
	
	return filesystem;
}

//currently uses first 386 blocks to initialize the filesystem
struct FileSystem* initializeFileSystem(char* file){
	printf("initializing \n");

	struct FileSystem* filesystem;
	char* map;

	//open/expand file, map memory, handle errors
	int fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (fd == -1){
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    } 
    lseek(fd, FOUR_MB, SEEK_SET);
    write(fd, "", 1);
	//map = mmap(NULL, FOUR_MB, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	map = mmap((void*) 0x7ffff7be0000, FOUR_MB, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0);
	if (map == MAP_FAILED){
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }

	//initialize the filesystem struct into the mapped memory
	filesystem = (struct FileSystem*)map;
	filesystem -> writeTo = map;
	filesystem -> map = map;

	//create a root sector (sector 0) 
	filesystem -> writeTo += sizeof(struct FileSystem);
	struct RootSector* sectorZero = (struct RootSector*)filesystem -> writeTo;
	filesystem -> writeTo += sizeof(struct RootSector);
	filesystem -> rootSec = sectorZero;
	
	//pad write pointer so sector 0 takes a whole block
	filesystem -> writeTo += PAGE_SIZE - (sizeof(struct FileSystem) + sizeof(struct RootSector));
	
	//create allocation table with reference in sector 0
	/*[cmt] FAT is the size of an integral number of pages now but beware of that changing*/
	struct FAT* allocationTable = (struct FAT*)filesystem -> writeTo;
	sectorZero -> allocationTable = allocationTable;
	filesystem -> writeTo += sizeof(struct FAT);
	
	//create root directory with reference in sector 0 and set filesystem current directory
	struct DirectoryPage* rootDirectory = (struct DirectoryPage*)filesystem -> writeTo;
	sectorZero -> rootDirectory = rootDirectory;
	filesystem -> writeTo += sizeof(struct DirectoryPage);
	strncpy(rootDirectory -> name, "root", NAME_MAX);
	filesystem -> currentDirectory = rootDirectory;
	
	//pad write pointer so root directory page takes a whole block
	filesystem -> writeTo += PAGE_SIZE - (sizeof(struct DirectoryPage));
	
	//reflect these structures in FAT
	//[cmt] this loop has to be modified if FAT size ever changes
	FATentry(filesystem, 'r', NULL, 0); // root sector
	struct PageNode* prev = FATentry(filesystem, 'f', NULL, 1);
	int i=2;
	for(i=2;i<=384;i++){
		prev = FATentry(filesystem, 'f', prev, i);
	}
	FATentry(filesystem, 'd', NULL, 385); //root directory
	
	//initialize counts
	filesystem -> blocksUsed = 386;
	filesystem -> fileCt = 0;
	
	filesystem -> rootSec -> currenttableposition = 386;
	
	return filesystem;
}

struct FileSystem* SequentialFATentry(struct FileSystem* filesystem, char type, struct PageNode* prev,unsigned short end){
	//ic ndex into allocation table
	struct PageNode* entry = filesystem -> rootSec -> allocationTable -> table;
	unsigned short i;
	printf("current free position: %d\n", filesystem->rootSec->currenttableposition);
	printf("end: %d\n", end);
	for(i=filesystem -> rootSec -> currenttableposition; i<(filesystem -> rootSec -> currenttableposition+end); i++){
	     entry[i].next=&entry[i+1];
	     /* if(entry[i-1]!=NULL){
	       entry[i]->prev=entry[i-1];
	       }*/
	//update attributes
	     entry[i].type = type;
	     entry[i].mapindex = i;
	}
	filesystem->rootSec->currenttableposition=filesystem -> rootSec -> currenttableposition+end;
	return filesystem;
	//printf("a: %d\n", filesystem->rootSec->currenttableposition);
	//return filesystem->rootSec->currenttableposition;
	//link from other nodes if necessary
	//if(prev){
	//	prev -> next = entry;
	     //	}
	
	//return entry;
}

//updates FAT entry for some page and links from previous if necessary. Argument prev is optional
struct PageNode* FATentry(struct FileSystem* filesystem, char type, struct PageNode* prev, unsigned short index){
	//index into allocation table
	struct PageNode* entry = filesystem -> rootSec -> allocationTable -> table;
	entry+=index;
	
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
	struct PageNode* cur = filesystem -> rootSec -> allocationTable -> table;

	while(cur->type){
		printf("This: %c %d ", cur->type, cur->mapindex);
		if(cur->next){
			printf("next: %c %d ", cur->next->type, cur->next->mapindex);
		}
		printf("  %p  \n", cur);
		cur++;
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