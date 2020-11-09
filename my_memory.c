// Include files
#include <stdio.h>
#include <stdlib.h>
#define  N_OBJS_PER_SLAB  64

// Global structs and variables
typedef struct Buddy {
  int free_or_allocated;
  void* address;
  struct Buddy* next;
} Buddy;
typedef struct free_list {
  int amt_avail;
  int size;
  Buddy* head;
} free_list;
typedef struct Slab {
  int* bits;
  int used;
  void* slab_start;
  struct Slab* next;
} Slab;
typedef struct Slabs {
  int per_slab_size;
  int total_size;
  int used;
  int slab_amt;
  struct Slab* first;
  struct Slabs* next;
} Slabs;
typedef struct Space {
  int size;
  int type;
  void* start;
} Space;
Slabs* head;
Space* space;
free_list* buddyList;

// Functional prototypes
void setup(int malloc_type, int mem_size, void* start_of_memory);
void* my_malloc(int size);
void my_free(void* ptr);
void* malloc_buddy(int size);
void free_buddy(void* ptr);

////////////////////////////////////////////////////////////////////////////
//
// Function     : setup
// Description  : initialize the memory allocation system
//
// Inputs       : malloc_type - the type of memory allocation method to be used [0..3] where
//                (0) Buddy System
//                (1) Slab Allocation

void setup(int malloc_type, int mem_size, void* start_of_memory) {
  int kb = 1024;
  space = (Space*)malloc(sizeof(Space));
  space->type = malloc_type;
  space->size = mem_size;
  space->start = start_of_memory;
  buddyList = (free_list*)malloc(sizeof(free_list) * (21));
  for(int i = 10; i <= 20; i++) { // 2^10 = 1024, 2^20 = 1mb
    buddyList[i].size = kb;
    buddyList[i].amt_avail = 0;
    buddyList[i].head = NULL;
    kb = kb * 2; // 2^index kb
  }
  buddyList[20].amt_avail++;
  buddyList[20].head = (Buddy*)malloc(sizeof(Buddy));
  buddyList[20].head->address = space->start;
  buddyList[20].head->next = NULL;
}

////////////////////////////////////////////////////////////////////////////
//
// Function     : my_malloc
// Description  : allocates memory segment using specified allocation algorithm
//
// Inputs       : size - size in bytes of the memory to be allocated
// Outputs      : -1 - if request cannot be made with the maximum mem_size requirement

void* my_malloc(int size) {
  if(space->type == 0) {
    return malloc_buddy(size);
  }
  int count = 0;
  Slabs* slabs;
  slabs = head;
  int newSize = size + 4; // for the header
  while((slabs != NULL) && (slabs->per_slab_size != newSize)) {
    slabs = slabs->next;
  }
  if((slabs == NULL) || (slabs->used == N_OBJS_PER_SLAB * slabs->slab_amt)) {
    Slabs* newSlabs;
    newSlabs = (Slabs*)malloc(sizeof(Slabs));
    newSlabs->per_slab_size = newSize;
    newSlabs->total_size = newSize * N_OBJS_PER_SLAB;
    newSlabs->slab_amt = 0;
    newSlabs->first = NULL;
    newSlabs->used = 0;
    newSlabs->next = head;
    head = newSlabs;
    slabs = head;
    void* slab_start;
    slab_start = malloc_buddy(slabs->total_size);
    Slab* newSlab;
    newSlab = (Slab*)malloc(sizeof(newSlab));
    newSlab->bits = (int*)malloc(sizeof(int) * N_OBJS_PER_SLAB);
    for(int i = 0; i < N_OBJS_PER_SLAB; i++) {
      newSlab->bits[i] = 0;
    }
    if(slab_start == NULL) {
      return NULL;
    }
    newSlab->used = 0;
    newSlab->slab_start = slab_start;
    newSlab->next = slabs->first;
    slabs->first = newSlab;
    slabs->slab_amt++;
  }
  Slab* slab2;
  slab2 = slabs->first;
  while((slab2->used == N_OBJS_PER_SLAB) && (slab2 != NULL)) {
    slab2 = slab2->next;
  }
  while((slab2->bits[count] == 1) && (count < N_OBJS_PER_SLAB)) {
    count++;
  }
  void* returnAddress;
  returnAddress = slab2->slab_start + (count * newSize);
  ((int*)returnAddress)[0] = newSize;
  returnAddress = returnAddress + 4;
  slabs->used++;
  slab2->used++;
  slab2->bits[count] = 1;
  return returnAddress;
}

////////////////////////////////////////////////////////////////////////////
//
// Function     : my_free
// Description  : deallocated the memory segment being passed by the pointer
//
// Inputs       : ptr - pointer to the memory segment to be free'd
// Outputs      :

void my_free(void* ptr) {
  if(space->type == 0) {
    return free_buddy(ptr);
  }
  Slabs* temp_head;
  temp_head = head;
  ptr = ptr - 4; // header
  int size = ((int*)ptr)[0];
  void* lastAddress;
  Slab* tempSlab;
  while((temp_head->per_slab_size != size) && (temp_head != NULL)) { 
    temp_head = temp_head->next;
  }
  if(temp_head == NULL) {
    return;
  }
  tempSlab = temp_head->first;
  while(tempSlab != NULL && (tempSlab->slab_start > ptr && ptr > lastAddress)) { 
    lastAddress = tempSlab->slab_start + N_OBJS_PER_SLAB * size; 
    tempSlab = tempSlab->next;
  }
  if(tempSlab == NULL) {
    return;
  }
  for(int i = 0; i < N_OBJS_PER_SLAB; i++) {
    lastAddress = tempSlab->slab_start + (i * size);
    if(lastAddress == ptr && tempSlab->bits[i] == 0) { 
      return;
    }
    if(lastAddress == ptr) {
      tempSlab->used--;
      temp_head->used--;
      tempSlab->bits[i] = 0;
      break;
    }
  }
  if(tempSlab->used == 0) {
    Slab* first = temp_head->first;
    if(first == tempSlab) {
      temp_head->first = first->next;
    } 
    if(first != tempSlab) {
      while(first->next != tempSlab) {
        first = first->next;
      }
      first->next = tempSlab->next;
    }
    temp_head->slab_amt--;
    free_buddy(tempSlab->slab_start);
  }
}

void* malloc_buddy(int size) {
  int foundSmallest = 0;
  int power;
  int newSize = size + 4; // for the header
  void* returnSmallestAddress = NULL;
  int count = 10; // 2^10 = 1024
  while(count <= 20) { // 2^20 = 1 mb
    if(buddyList[count].size >= newSize) {
      if(foundSmallest == 0) {
        power = count;
        foundSmallest = 1;
      }
      if(buddyList[count].amt_avail > 0) {
        Buddy* index;
        Buddy* temp;
        index = buddyList[count].head;
        temp = buddyList[count].head;
        void* smallestAddress = space->size + space->start + 1;
        while(index != NULL) {
          if(index->free_or_allocated == 0) {
            if(index->address < smallestAddress) {
              smallestAddress = index->address;
              temp = index;
            }
          }
          index = index->next;
        }
        // remove from free_list
        if(temp == buddyList[count].head) {
          buddyList[count].head = buddyList[count].head->next;
        } 
        else if(temp != buddyList[count].head) {
          Buddy* newTemp;
          newTemp = buddyList[count].head;
          while(newTemp->next != NULL && newTemp->next != temp) {
            newTemp = newTemp->next;
          }
          if(newTemp->next == temp) {
            newTemp->next = newTemp->next->next;
          }
        }
        returnSmallestAddress = smallestAddress + 4; // header
        buddyList[count].amt_avail--;
        Buddy* busyBuddy = (Buddy*)malloc(sizeof(Buddy));
        busyBuddy->address = smallestAddress;
        busyBuddy->next = buddyList[power].head;
        busyBuddy->free_or_allocated = 1;
        buddyList[power].head = busyBuddy;
        smallestAddress = smallestAddress + buddyList[power].size;
        int count2 = power;
        while(count2 < count) {
          Buddy* freeBuddy = (Buddy*)malloc(sizeof(Buddy));
          freeBuddy->free_or_allocated = 0;
          freeBuddy->next = buddyList[count2].head;
          freeBuddy->address = smallestAddress;
          buddyList[count2].amt_avail++;
          buddyList[count2].head = freeBuddy;
          smallestAddress = smallestAddress + buddyList[count2].size;
          count2++;
        }
        break;
      }
    }
    count++;
  }
  return returnSmallestAddress;
}

void free_buddy(void* ptr) {
  int stop = 0;
  int powerCount = 10;
  int power;
  Buddy* to_be_freed;
  Buddy* temp;
  ptr = ptr - 4;
  while(powerCount <= 20 && stop == 0) { 
    temp = buddyList[powerCount].head;
    while(temp != NULL && stop == 0) {
      if(temp->address == ptr) {
        to_be_freed = temp;
        to_be_freed->free_or_allocated = 0;
        stop = 1;
        power = powerCount;
      }
      if(stop == 0) {
        temp = temp->next;
      }
    }
    powerCount++;
  }
  Buddy* erase = buddyList[power].head;
  if(buddyList[power].head == to_be_freed) {
    buddyList[power].head = buddyList[power].head->next;
  }
  else if(buddyList[power].head != to_be_freed) {
    while(erase->next != to_be_freed && erase->next != NULL) {
      erase = erase->next;
    }
    erase->next = erase->next->next;
  }
  Buddy* newTemp;
  int brake = 0;
  while(power <= 20 && brake == 0) {
    int size = buddyList[power].size;
    unsigned long remainder = (ptr - space->start) % (size * 2);
    void* address;
    if(remainder == 0) {
      address = ptr + size;
    }
    if(remainder == size) {
      address = ptr - size;
    }
    newTemp = buddyList[power].head;
    int brk = 0;
    while(newTemp != NULL && brk == 0) {
      if(newTemp->address == address) {
        brk = 1;
      }
      if(brk == 0) {
        newTemp = newTemp->next;
      }
    }
    if(newTemp == NULL || newTemp->free_or_allocated == 1) { 
      to_be_freed->address = ptr;
      to_be_freed->next = buddyList[power].head;
      to_be_freed->free_or_allocated = 0;
      buddyList[power].head = to_be_freed;
      buddyList[power].amt_avail++;
      brake = 1;
    }
    else {
      erase = buddyList[power].head;
      if(buddyList[power].head == newTemp) {
        buddyList[power].head = buddyList[power].head->next;
      } 
      else {
        while(erase->next != newTemp) {
          erase = erase->next;
        }
        erase->next = erase->next->next;
      }
      buddyList[power].amt_avail--;
      if(address < ptr) {
        ptr = address;
      }
    }
  power++;
  }
}
