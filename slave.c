#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "config.h"

int main(const int argc, char * argv[]){
  int i;
  char logname[50];

  if(argc != 2){
    fprintf(stderr, "Error: Missing argument id\n");
    return EXIT_FAILURE;
  }

  const int id = atoi(argv[1]);

  //initialize rand() function
  srand(time(NULL));
  //don't buffer stdout
  setvbuf(stdout, NULL, _IONBF, 0);

  //redirect output to file
  snprintf(logname, sizeof(logname), "logfile.%d", id);
  stdout = freopen(logname, "w", stdout);


  if(bakery_initialize(0) == -1){
    fprintf(stderr, "Error: slave failed to initialize\n");
    return 1;
  }

  for(i=0; i < 5; i++){

    printf("[%s] lock\n", timestring());
    bakery_lock(id);
    printf("[%s] locked\n", timestring());

    //do the sleeping
    const int sleep_time = 1;//  + (rand() % 4);
    sleep(sleep_time);

    const int q = bakery_queue_size(id);

    FILE * fp = fopen("cstest", "a");
    if(fp == NULL){
      perror("fopen");
      bakery_unlock(id);
      break;
    }
    fprintf(fp, "[%s] Queue %d File modified by process number %d\n", timestring(), q, id);
    fclose(fp);

    fprintf(stdout, "[%s] Queue %d File modified by process number %d\n", timestring(), q, id);

    sleep(sleep_time);

    printf("[%s] unlock\n", timestring());
    bakery_unlock(id);
    printf("[%s] unlocked\n", timestring());
  }

  bakery_deinitialize();

  return 0;
}
