/***********************************************************************/
/**      Author: Minas Spetsakis                                      **/
/**        Date: June 2021                                            **/
/** Description: Skeleton for Assgn. II                               **/
/***********************************************************************/

#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "queue.h"
#include "args.h"
#include "error.h"

int njobs,			/* number of jobs ever created */
  ttlserv,			/* total service offered by servers */
  ttlqlen;			/* the total qlength  */
int nblocked;    /* The number of threads blocked */
	

/***********************************************************************
                           r a n d 0 _ 1
************************************************************************/
double rand0_1(unsigned int *seedp)
{
  double f;
  /* We use the re-entrant version of rand */
  f = (double)rand_r(seedp);
  //printf("test %f\n", f/(double)RAND_MAX);
  return f/(double)RAND_MAX;
}


/***********************************************************************
                             C L I E N T
************************************************************************/

void *client(void *vptr)
{
  unsigned int seed;
  int pthrerr;
  struct thread_arg *ptr;
 
  ptr = (struct thread_arg*)vptr;


  printf("%f\n", ptr->lam);
  

printf("Hello from client thread \n");
  while (1)
    {
      //double randNum = rand0_1(&seed);
      //printf("seed %f lam %f\n", randNum,  ptr->lam);
      
      //if (randNum < ptr->lam){
      //  printf("im smaller than lam! %f < %f\n", randNum, ptr->lam);
      //}
      //push(ptr->q)
      //mutex lock blocktex 
      //update nblocked++
      // unlock

    }
  return NULL;
      
}

/***********************************************************************
                             S E R V E R
************************************************************************/

void *server(void *vptr)
{
  int busy;
  int pthrerr;
  struct thread_arg *ptr;

  ptr = (struct thread_arg*)vptr;

  busy = 0;

  printf("Hello from server thread \n");
  printf("mu %f \n", ptr->mu);

  while (1)
    {
      //mutex lock blocktex 
      //update nblocked++
      // unlock
      //block(wait)

    }
  return NULL;
}


/***********************************************************************
                                C L K
************************************************************************/

void *clk(void *vptr)
{
  int tick;
  int pthrerr;
  struct thread_arg *ptr;

  ptr = (struct thread_arg*)vptr;

  //if blocked == 2 
  // tick++
  // lock blocktex
  // nblocked = 0
  // unlock 
  // wake up (signal cond) blocked sever & client


  printf("Average waiting time:    %f\n",(float)ttlqlen/(float)njobs);
  printf("Average turnaround time: %f\n",(float)ttlqlen/(float)njobs+
	 (float)ttlserv/(float)njobs);
  printf("Average execution time:  %f\n",(float)ttlserv/(float)njobs);
  printf("Average queue length: %f\n",(float)ttlqlen/(float)ptr->nticks);
  printf("Average interarrival time time: %f\n",(float)ptr->nticks/(float)njobs);
  /* Here we die with mutex locked and everyone else asleep */
  exit(0);
}

int main(int argc, char **argv)
{
  int pthrerr, i;
  int nserver, nclient, nticks;
  float lam, mu;

  pthread_t server_tid, client_tid;
  pthread_cond_t sthrblockcond, sclkblockcond;
  pthread_mutex_t sblocktex, sstatex;
  struct thread_arg *allargs;
  pthread_t *alltids;

  pthread_t servers[nserver], clients[nclient];

  ttlserv  = 0;
  ttlqlen  = 0;
  nblocked = 0;
  njobs    = 0;

  nserver = 2;
  nclient = 2;
  lam = 0.005;
  mu = 0.01;
  nticks = 1000;
  i=1;
  while (i<argc-1)
    {
      if (strncmp("--lambda",argv[i],strlen(argv[i]))==0)
	      lam = atof(argv[++i]);
      else if (strncmp("--mu",argv[i],strlen(argv[i]))==0)
	      mu = atof(argv[++i]);
      else if (strncmp("--servers",argv[i],strlen(argv[i]))==0)
	      nserver = atoi(argv[++i]);
      else if (strncmp("--clients",argv[i],strlen(argv[i]))==0)
	      nclient = atoi(argv[++i]);
      else if (strncmp("--ticks",argv[i],strlen(argv[i]))==0)
	      nticks = atoi(argv[++i]);
      else
	      fatalerr(argv[i], 0, "Invalid argument\n");
         i++;
    }

  if (i!=argc)
    fatalerr(argv[0], 0, "Odd number of args\n");

  allargs = (struct thread_arg *)malloc((nserver+nclient+1)*sizeof(struct thread_arg));
  if (allargs==NULL)
    fatalerr(argv[0], 0,"Out of memory\n");

  alltids = (pthread_t*)malloc((nserver+nclient)*sizeof(pthread_t));
  if (alltids==NULL)
    fatalerr(argv[0], 0,"Out of memory\n");
  


  //filling args with values
  allargs->nserver = nserver;
  allargs->nclient = nclient;
  allargs->nticks = nticks;
  allargs->lam = lam;
  allargs->mu = mu;

  printf("lam %f \n",allargs->lam );

  //initializing mutexs & conds and pointing struct mutex & conds to them
  pthrerr = pthread_mutex_init(&sblocktex, NULL);
  if (pthrerr!=0)
    fatalerr("Main",pthrerr,"blocktex initialization failed\n");
  allargs->blocktex = &sblocktex;
  pthrerr = pthread_mutex_init(&sstatex, NULL);
  if (pthrerr!=0)
    fatalerr("Main",pthrerr,"statex initialization failed\n");
  allargs->statex = &sstatex;
  pthrerr = pthread_cond_init(&sthrblockcond, NULL);
  if (pthrerr!=0)
    fatalerr("Main",pthrerr,"thrblockcond initialization failed\n");
  allargs->thrblockcond = &sthrblockcond;
  pthrerr = pthread_cond_init(&sclkblockcond, NULL);
  if (pthrerr!=0)
    fatalerr("Main",pthrerr,"clkblockcond initialization failed\n");
  allargs->clkblockcond = &sclkblockcond;

  //initialize q
  allargs->q = mk_queue();

  //creating client and server threads
  for(int j = 0; j < nclient; j++){
  pthrerr = pthread_create(&clients[j], NULL, &client, &allargs);
  if (pthrerr!=0)
    fatalerr("Main",pthrerr,"Failed to create client thread 1\n");
  }

  for(int j = 0; j < nserver; j++){
  pthrerr = pthread_create(&servers[j], NULL, &server, &allargs);
  if (pthrerr!=0)
    fatalerr("Main",pthrerr,"Failed to create server thread 1\n");
  }

//calling clk function
clk(&allargs);

//joining client and server thread
for(int j = 0; j < nserver; j++){
pthrerr = pthread_join(servers[j], NULL);
if (pthrerr!=0)
    fatalerr("Main",pthrerr,"Failed to join server thread 1\n");
}

for(int j = 0; j < nclient; j++){
pthrerr = pthread_join(clients[j], NULL);
if (pthrerr!=0)
    fatalerr("Main",pthrerr,"Failed to join client thread 1\n");
}

//destroy mutex & conds
pthread_mutex_destroy(&sblocktex);
pthread_mutex_destroy(&sstatex);
pthread_cond_destroy(&sthrblockcond);
pthread_cond_destroy(&sclkblockcond);

  exit(-1);
}
