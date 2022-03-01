#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

static int shmid = -1;
static struct bakery_alg * shmem = NULL;

//Find the current maximum number
static int maximum(void){
  int i, j = 0; //assume max numer is at index 0

  //start from index 1 and search for the maximum number
  for (i = 1; i < shmem->nn; i++) {
    if(shmem->numbers[i] > shmem->numbers[j]){
      j = i;  //update maximum number index
    }
  }
  return shmem->numbers[j];
}

//Initialize the shared memory
int bakery_initialize(const int nn){

  //if we are the master ( nn > 0), use create shared memory flags
  const int shmflg = (nn > 0) ? IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR : 0;

  //get the memory
  shmid = shmget(ftok("master.c", 4343), sizeof(struct bakery_alg), shmflg);
  if(shmid == -1){
    perror("shmget");
    return -1;
  }

  //attach to the pointer
  shmem = (struct bakery_alg*) shmat(shmid, NULL, 0);
  if(shmem == (void*)-1){
    perror("shmat");
    return -1;
  }

  if(nn){ //if we are creating
    bzero(shmem, sizeof(struct bakery_alg));

    //set the number of processes
    shmem->nn = nn;
    //save our PID, so we know who to clear it later
    shmem->master_pid = getpid();
  }

  return 0;
}

//Deinitialize the shared memory
int bakery_deinitialize(){

  if(shmid != -1){  //if its initialized
    //and this process created it
    if(shmem->master_pid == getpid()){
      shmdt(shmem);
      shmctl(shmid, IPC_RMID, NULL);
    }else{
      shmdt(shmem);
    }
  }

  return 0;
}

//Lock critical region using bakery algorithm
void bakery_lock(const int id){
  int i;

  shmem->choosing[id] = 1;
  shmem->numbers[id]  = maximum() + 1;
  shmem->choosing[id] = 0;

  for (i=0; i < shmem->nn; i++) {
    if(i != id){
      while (shmem->choosing[i] == 1);
      while ((shmem->numbers[i] != 0) && ((shmem->numbers[i] < shmem->numbers[id]) || ((shmem->numbers[i] == shmem->numbers[id]) && (i < id))));
    }
  }
}

//Unlock critical region using bakery algorithm
void bakery_unlock(const int id){
  shmem->numbers[id] = 0;
}

//Count how many processes are waiting in queue
int bakery_queue_size(const int id){
  int i, len = 0;
  for (i = 0; i < shmem->nn; i++) {
    if(shmem->numbers[i] != 0){ //if process has a number, its waiting
      len++;
    }
  }

  return len - 1; //-1 since 1 has locked the cirtical section and is using it
}

//Generate a time string
char* timestring(){
  static char buf[50];

  time_t t = time(NULL);
  struct tm * tmbuf = localtime(&t);
  if (tmbuf == NULL) {
    perror("localtime");
    return NULL;
  }

  if (strftime(buf, sizeof(buf), "%H:%M:%S", tmbuf) == 0) {
     fprintf(stderr, "Error: strftime failed");
     return NULL;
  }

  return buf;
}
