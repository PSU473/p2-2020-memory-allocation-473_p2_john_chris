// project2.c
// Author: Ata Fatahi
//mailto: azf82@psu.edu
//Description: Parser and Handler for Project 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdint.h>

extern void setup( int malloc_type, int mem_size, void* start_of_memory );
extern void *my_malloc( int size );
extern void my_free( void *ptr );

FILE *fd;
struct handle{
  char * _handle;
  int ** addresses;
  struct handle * next;
};
typedef struct handle handle_t;

struct ops{
  int numops ;
  char *handle;
  char *type;
  int size ;
};
typedef struct ops ops_t;

int open_file(char *filename)
{
	char mode = 'r';
	fd = fopen(filename, &mode);
	if (fd == NULL)
	{
		printf("Invalid input file specified: %s\n", filename);
		return -1;
	}
	else
	{
		return 0;
	}
}

void close_file()
{
	fclose(fd);
}

int  read_next_ops(ops_t * op){
  char line[1024];
  if(!fgets(line, 1024, fd)) {
    return 0;
  }

  char *token;
  char* rest=line;
if(token=strtok_r(rest, " ", &rest)) {
  strcpy(op->handle, token);
} else {
  return 0;
}
if(token=strtok_r(rest, " ", &rest)) {
  op->numops = atof(token);
} else {
  return 0;
}
if(token=strtok_r(rest, " ", &rest)) {
  strcpy(op->type, token);
} else {
  return 0;
}
if(token=strtok_r(rest, " ", &rest)) {
  op->size = atof(token);
} else {
  return 0;
}
return 1;
}
int main(int argc, char *argv[]){


  if (argc < 3)
  {
    printf ("Not enough parameters specified.  Usage: a.out <allocation_type> <input_file>\n");
    printf ("  Allocation type: 0 - Buddy System\n");
    printf ("  Allocation type: 1 - Slab Allocation\n");
    return -1;
  }

  if (open_file(argv[2]) < 0){
    return -1;
  }


  int size;
  int RAM_SIZE=1<<20;//1024*1024
  void* RAM=malloc(RAM_SIZE);//1024*1024
	setup(atoi(argv[1]),RAM_SIZE,RAM);


  handle_t * handles = NULL;
  ops_t * op = (ops_t*) malloc(sizeof(ops_t));
  op->type = (char*) malloc(sizeof(char)*2);
  op->handle = (char*) malloc(sizeof(char)*2);
  while(read_next_ops(op)){
    if(strcmp(op->type ,"M")==0){//do my_alloc
      if (handles == NULL){
        handles = (handle_t *) malloc(sizeof(handle_t));
        handles->_handle = op->handle;
        handles->addresses = (int**)malloc(sizeof(int*)*op->numops);
        handles->next = NULL;
        for (int i=0; i< op->numops; i++){
            void * x = my_malloc(op->size);
            if((intptr_t)x == -1){
              printf("Allocation Error\n" );
            }
            else
            {
              *(handles->addresses+i) = (int*)x;
            printf("Start of Chunk is: %d\n", (int)((void*)(*(handles->addresses+i)) - RAM));
          }
          }
      }
      else{
        handle_t * hp = handles;
        while (hp->next != NULL) hp = hp->next;
        hp->next = (handle_t *) malloc(sizeof(handle_t));
        hp->next->_handle = op->handle;
        hp->next->addresses =  (int**)malloc(sizeof(int*)*op->numops);
        for (int i=0; i< op->numops; i++){

          void * x = my_malloc(op->size);
          if((intptr_t)x == -1){
            printf("Allocation Error\n" );
          }
          else
          {
        *(hp->next->addresses+i) = (int*)x;
        printf("Start of Chunk is: %d\n", (int)((void*)(*(hp->next->addresses+i)) - RAM));
      }
      }
      }
    }
    else if(strcmp(op->type ,"F")==0){//do free
      handle_t * hp = handles;
      while(hp !=NULL){
        if (strcmp(hp->_handle, op->handle)==0){
          int index = op->numops;
          my_free((void *)(*(hp->addresses+index)));
          printf("freed object at %d\n",  (int)((void*)(*(hp->addresses+index)) - RAM));
          break;
        }
      }
    }
    else{
      printf("Incorrect input file content\n");
      return 0;
    }

  }


  return 0;
}
