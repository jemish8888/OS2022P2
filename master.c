#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#include "config.h"

//Array to hold slave PIDs
static pid_t *pids = NULL;
static int nn = 0;

static void signal_handler(int sig){
  int i;

  switch(sig){
    case SIGALRM:
      printf("[%s] master: ALRM signal\n", timestring());
      break;
    case SIGINT:
      printf("[%s ]master: INT signal\n", timestring());
      break;
    default:
      printf("[%s ]master: unknown signal\n", timestring());
      break;
  }

  //send termination to all user processes
  printf("\tterminating %d slave processes\n", nn);
  for(i=0; i < nn; i++){
    if(kill(pids[i], SIGINT) == -1){
      perror("kill");
    }
  }
}

int main(const int argc, char * argv[]){

  struct sigaction sa;
  int i, opt, ss = DEFAULT_ALARM;
  char arg[10];

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = signal_handler;
  if( (sigaction(SIGALRM, &sa, NULL) == -1) ||
      (sigaction(SIGINT,  &sa, NULL) == -1) ){
   perror(argv[0]);
   return 1;
  }

  stdout = freopen("logfile", "w", stdout);

  while( (opt = getopt(argc, argv, "t:")) > 0){
    switch(opt){
      case 't': ss = atoi(optarg);  break;
      default:
        fprintf(stderr, "Error: Unknown argument %c\n", opt);
        return EXIT_FAILURE;
    }
  }

  //check if we have nn argument
  if(optind > (argc - 1)){
    fprintf(stderr, "Error: Missing argument nn\n");
    return EXIT_FAILURE;
  }

  //make sure nn is not above maximum
  if(atoi(argv[optind]) >= SLAVE_COUNT){
    nn = SLAVE_COUNT;
  }else{
    nn = atoi(argv[optind]);
  }

  pids = (pid_t*) malloc(sizeof(pid_t)*nn);
  if(pids == NULL){
    perror("malloc");
    return EXIT_FAILURE;
  }

  if(bakery_initialize(nn) == -1){
    fprintf(stderr, "Error: Failed to initialize shared memory\n");
    return 1;
  }


  for(i=0; i < nn; i++){

    const pid_t pid = fork();
    switch(pid){
      case -1:
        perror(argv[0]);
        break;

      case 0:
        snprintf(arg, sizeof(arg), "%d", i);

        execl("./slave", "./slave", arg, NULL);
        perror(argv[0]);
        return EXIT_FAILURE;
        break;

      default:
        pids[i] = pid;
        printf("[%s] master: process %d started\n", timestring(), pid);
        break;
    }
  }

  alarm(ss);

  for(i=0; i < nn; i++){
    int wstatus = 0;
    do {
      const pid_t w = waitpid(-1, &wstatus, WUNTRACED | WCONTINUED);
      if (w == -1) {
         perror(argv[0]);
         return EXIT_FAILURE;
      }

      if (WIFEXITED(wstatus)) {
         printf("[%s] master: %d exited with code %d\n", timestring(), w, WEXITSTATUS(wstatus));

      } else if (WIFSIGNALED(wstatus)) {
         printf("[%s] master: %d signalled with code %d\n", timestring(), w, WTERMSIG(wstatus));
      }
    } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
  }

  //cleanup
  bakery_deinitialize();

  free(pids);

  return 0;
}
