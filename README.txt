1. make the programs
  $ make
gcc -Wall -g -c bakery.c
gcc -Wall -g -o master master.c bakery.o
gcc -Wall -g -o slave slave.c bakery.o

2. Run the programs

  $ ./master -t 100 5

  # Parameters can have any order
  $ ./master 5 -t 100
