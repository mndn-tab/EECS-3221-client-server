# Probabilistic Simulation of a Client-Server system with C

A simple simulation of a client-server system using pthreads, mutex' and condition variables
in C Programming Language. 

The command line options to the program are the --lambda [0.005], --mu [0.01],
--servers[2], --clients[2], --ticks[1000].

The client threads:  takes in one parameter, the birth rate λ (lambda) of the jobs.
The client uses the random number generator to decide if it creates a new job.
If the random number is smaller than λ a new job is generated and placed on the global queue.

The server threads: need one parameter, the completion rate µ (mu), also called death rate,
of the currently executing process. At every activation of the server, if there is a job 
executing, the server generates a random number between zero and one and compares it to µ.
If the random number is smaller than µ the job execution is terminated. If there is no job
executing, the server gets a job from the global queue if available. The state of the server
can be either busy or not-busy.

The clock thread:  At every tick of the clock thread, all other threads get activated, do whatever 
they need to do and then wait for the next tick. Whenever the clock sees that all the threads have 
completed their work for the current tick, it emits the next tick. The program runs for the
number of ticks specified.

Statistics are updated at every "tick" and printed once all ticks are completed:

(1) Average waiting time (AWT)
(2) Average execution time (AXT)
(3) Average turnaround time (ATA)
(4) Average queue length (AQL)
(5) Average interarrival time. (AIA)

# To run

simul: simul.o queue.o error.o
	gcc simul.o queue.o error.o -pthread -lc -o simul

simul.o: simul.c args.h queue.h error.h
	gcc -c -pthread simul.c

queue.o: queue.c queue.h error.h
	gcc -c -pthread queue.c

error.o: error.c error.h
	gcc -c error.c

clean:
	/bin/rm simul.o queue.o error.o simul

tar:
	tar cf Simul.tar *.c *.h makefile Marking

# To test
Tested:
  with no arguments
  --clients 10 --servers 12 --lambda 0.01 --mu 0.01
  --clients 20 --servers 20 --lambda 0.01 --mu 0.011


Simul$ ./simul --clients 10 --servers 12 --lambda 0.01 --mu 0.01
Average waiting time:    6.454545
Average execution time:  85.090912
Average turnaround time: 91.545456
Average queue length: 0.639000
Average interarrival time time: 10.101010


Simul$ ./simul --clients 20 --servers 20 --lambda 0.01 --mu 0.011
Average waiting time:    53.752213
Average execution time:  77.933632
Average turnaround time: 131.685852
Average queue length: 12.148000
Average interarrival time time: 4.424779
