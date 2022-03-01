#ifndef CONFIG_H
#define CONFIG_H

//Maximum number of processes
#define SLAVE_COUNT 20

#define DEFAULT_ALARM 100

#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "config.h"

struct bakery_alg {
  //number of processes
  int nn;

  //bakery ticket numbers
  int  numbers[SLAVE_COUNT];
  //processes choosing a number
  int choosing[SLAVE_COUNT];

  //who created the shared memory
  pid_t master_pid;
};

//Create and destroy functions
int bakery_initialize(const int nn);
int bakery_deinitialize();

//// synchronization functions
//lock and unlock
void bakery_lock(const int id);
void bakery_unlock(const int id);

////Helper functions

//get queue size
int  bakery_queue_size(const int id);
//generate a time buffer
char* timestring();

#endif
