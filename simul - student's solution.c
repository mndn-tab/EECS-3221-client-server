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
int clientsWaiting;


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

void *client(void *vptr){
  unsigned int seed;
  int pthrerr;
  struct thread_arg *ptr;
  double randNum;
  int waitingForService = 0;

  ptr = (struct thread_arg*)vptr;

  pthread_mutex_lock(ptr->blocktex);
  seed=ptr->seed;

  //printf("Hello from client thread with lam %f \n", ptr->lam);
  //printf("thread-  allargs memory %p \n", ptr );

  while (1){

    if (waitingForService == 0){
    //printf("no jobs waiting for service completion. attempting to generate new job...\n");
    randNum = rand0_1(&seed);
    //printf("seed %f lam %f\n", randNum,  ptr->lam);
      if (randNum < ptr->lam){
        //printf(" Adding job... %f < %f\n", randNum, ptr->lam);
        waitingForService = 1;
        push_q(ptr->q); 
        //printf("we are now waiting for service toc complete on my job %d.\n", waitingForService);
        pthread_mutex_lock(ptr->statex);
        njobs++;
        ttlqlen = size_q(ptr->q);
        pthread_mutex_unlock(ptr->statex);
        clientsWaiting++;
        nblocked++;
        //printf("clients waiting: %d , nblocked: %d\n", clientsWaiting, nblocked);
        //printf("nblocked is %d and total is %d\n",nblocked,ptr->nserver+ptr->nclient);
        if(nblocked == ptr->nserver+ptr->nclient){
          pthread_cond_signal(ptr->clkblockcond);
          //printf("server sent signal to clock.\n");

          pthread_cond_wait(ptr->waitingcond, ptr->blocktex); 
        } else {
          //printf("not the last client to block. waiting for tick.\n");

          pthread_cond_wait(ptr->waitingcond, ptr->blocktex);
        }

        //printf("Thank you for completing my job.\n");
        waitingForService = 0;

      } else {
        nblocked++;
        //printf("no job is generated for this activation. blocked.\n");

        if(nblocked == ptr->nserver+ptr->nclient){
          pthread_cond_signal(ptr->clkblockcond);
          //printf("signal sent to clk 2 by client\n");
          pthread_cond_wait(ptr->thrblockcond, ptr->blocktex); 
          //printf("client is awake...\n");
        } else {
          //printf("not last one to block. waiting for tick...\n");
          pthread_cond_wait(ptr->thrblockcond, ptr->blocktex); 
          //printf("2 client is awake...\n");
        }
      }

    } else if (waitingForService ==1) {
      //printf("Cannot generate another job until someone completes my existing one.\n");
      nblocked++;
              //printf("clients waiting: %d , nblocked: %d\n", clientsWaiting, nblocked);

      if(nblocked == ptr->nserver+ptr->nclient){
        pthread_cond_signal(ptr->clkblockcond);
        pthread_cond_wait(ptr->waitingcond, ptr->blocktex); 
      } else {
        pthread_cond_wait(ptr->waitingcond, ptr->blocktex); 
      }

      //printf("Thank you for completing my job.\n");
      waitingForService = 0;
    }
  }
  
  return NULL;    
}

/***********************************************************************
                             S E R V E R
************************************************************************/

void *server(void *vptr){
  unsigned int seed;
  int busy;
  int pthrerr;
  struct thread_arg *ptr;

  ptr = (struct thread_arg*)vptr;

  seed=ptr->seed;
  busy = 0;
  pthread_mutex_lock(ptr->blocktex);
  //printf("Hello from server thread \n ");
  //printf("sever  allargs memory %p \n", ptr );

  while (1){
    if (busy==0){
      if(size_q(ptr->q) > 0){
      //  printf("%d Jobs waiting...\n",size_q(ptr->q));
        pop_q(ptr->q);
        busy = 1;
      //  printf("Took on a job. %d jobs waiting... \n",size_q(ptr->q));
      } else {
      //  printf("no jobs to take for this activation. BLOCKED.\n");
        nblocked++;
              //  printf("clients waiting: %d , nblocked: %d\n", clientsWaiting, nblocked);

        if(nblocked == ptr->nserver+ptr->nclient){
          pthread_cond_signal(ptr->clkblockcond);
        //  printf("signal sent to clk 1 by seerver \n");
          pthread_cond_wait(ptr->thrblockcond, ptr->blocktex); 
        //  printf("1 server is awake...\n");
        } else {
        //  printf("not last one to block. waiting for tick...\n");
          pthread_cond_wait(ptr->thrblockcond, ptr->blocktex); 
        //  printf("2 server is awake...\n");
        }
      }
    } else if(busy == 1){
      double randNum = rand0_1(&seed);
      //printf("seed %f mu %f busy? %d\n", randNum,  ptr->mu, busy);
      if (randNum < ptr->mu){

        busy = 0;
        //printf("clients waiting: %d , nblocked: %d\n", clientsWaiting, nblocked);

        //printf("Job terminated, waiting for next job. busy? %d\n", busy);
        clientsWaiting--;
                      //printf("clients waiting: %d , nblocked: %d\n", clientsWaiting, nblocked);


        nblocked++;
        //printf("nblocked is %d and total is %d\n",nblocked,ptr->nserver+ptr->nclient);
        pthread_cond_signal(ptr->waitingcond);
        if(nblocked == ptr->nserver+ptr->nclient){
          pthread_cond_signal(ptr->clkblockcond);
        //printf("signal sent to clk 4 by server\n");
          pthread_cond_wait(ptr->thrblockcond, ptr->blocktex); 
        //printf("3 server is awake...\n");
        } else {
        //printf("not last one to block. waiting for tick...\n");
          pthread_cond_wait(ptr->thrblockcond, ptr->blocktex); 
        //printf("4 server is awake...\n");
        }
              
      } else {
        pthread_mutex_lock(ptr->statex);
        ttlserv++;
        pthread_mutex_unlock(ptr->statex);
      //printf("contuing on same job as before for this activation. blocked\n");
        nblocked++;          
        if(nblocked == ptr->nserver+ptr->nclient){
          pthread_cond_signal(ptr->clkblockcond);
        //printf("signal sent to clk 5\n");
          pthread_cond_wait(ptr->thrblockcond, ptr->blocktex); 
        //printf("5 server is awake...\n");
        } else {
        //printf("not last one to block. waiting for tick...\n");
          pthread_cond_wait(ptr->thrblockcond, ptr->blocktex); 
        //printf("6 server is awake...\n");
        }
      }
    }
  }
  return NULL;
}


/***********************************************************************
                                C L K
************************************************************************/

void *clk(void *vptr){
  int tick = 0;
  int pthrerr;
  struct thread_arg *ptr;


  ptr = (struct thread_arg*)vptr;
  pthread_mutex_lock(ptr->blocktex);
  //printf("clk  allargs memory %p \n", ptr );

  while(tick < ptr->nticks){

    pthread_cond_wait(ptr->clkblockcond, ptr->blocktex); 
    //printf("clock awoken 1\n");
    //printf("clients waiting: %d , nblocked: %d\n", clientsWaiting, nblocked);
    if(clientsWaiting > 0){
    nblocked=clientsWaiting;
    } else {
      nblocked = 0;
    }
    //printf("clients waiting: %d , nblocked: %d\n", clientsWaiting, nblocked);
    //printf("block reset and waking up everyone\n");
    pthread_cond_broadcast(ptr->thrblockcond); 
    tick++;
    if(tick == ptr->nticks){
      pthread_cond_broadcast(ptr->waitingcond); 

    }
    //printf("clock going back to sleep tick %d\n",tick);
  }

  printf("Average waiting time:    %f\n",(float)ttlqlen/(float)njobs);
  printf("Average turnaround time: %f\n",(float)ttlqlen/(float)njobs+
	 (float)ttlserv/(float)njobs);
  printf("Average execution time:  %f\n",(float)ttlserv/(float)njobs);
  printf("Average queue length: %f\n",(float)ttlqlen/(float)ptr->nticks);
  printf("Average interarrival time time: %f\n",(float)ptr->nticks/(float)njobs);

  printf("Total jobs ever created: %f\n",(float)njobs);
  printf("Total time spent servicing: %f\n",(float)ttlserv);



  /* Here we die with mutex locked and everyone else asleep */

  //destroy mutex & conds
  pthread_mutex_destroy(ptr->blocktex);
  pthread_mutex_destroy(ptr->statex);
  pthread_cond_destroy(ptr->thrblockcond);
  pthread_cond_destroy(ptr->clkblockcond);
  pthread_cond_destroy(ptr->waitingcond);

  free_queue(ptr->q);
  exit(0);
}

int main(int argc, char **argv)
{
  int pthrerr, i;
  int nserver, nclient, nticks;
  float lam, mu;

  pthread_t server_tid, client_tid;
  pthread_cond_t sthrblockcond, sclkblockcond, swaitingcond;
  pthread_mutex_t sblocktex, sstatex;
  struct thread_arg *allargs;
  pthread_t *alltids;

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
  
 //initializing mutexs & conds 
  pthrerr = pthread_mutex_init(&sblocktex, NULL);
  if (pthrerr!=0)
    fatalerr("Main",pthrerr,"blocktex initialization failed\n");
  pthrerr = pthread_mutex_init(&sstatex, NULL);
  if (pthrerr!=0)
    fatalerr("Main",pthrerr,"statex initialization failed\n");
  pthrerr = pthread_cond_init(&sthrblockcond, NULL);
  if (pthrerr!=0)
    fatalerr("Main",pthrerr,"thrblockcond initialization failed\n");
  pthrerr = pthread_cond_init(&sclkblockcond, NULL);
  if (pthrerr!=0)
    fatalerr("Main",pthrerr,"clkblockcond initialization failed\n");
  pthrerr = pthread_cond_init(&swaitingcond, NULL);
  if (pthrerr!=0)
    fatalerr("Main",pthrerr,"waitingcond initialization failed\n");

  //initialize q
  queue_t *q = mk_queue();

  //generate random numbers for seed
  srand(time(0));

  //CREATING CLIENTS
  int j;
  for(j = 0; j <= nserver + nclient; j++){

  //filling args with values
  (allargs+j)->nserver = nserver;
  (allargs+j)->nclient = nclient;
  (allargs+j)->nticks = nticks;
  (allargs+j)->lam = lam;
  (allargs+j)->mu = mu;

  (allargs + j)->blocktex = &sblocktex;
  (allargs + j)->statex = &sstatex;
  (allargs + j)->thrblockcond = &sthrblockcond;
  (allargs + j)->clkblockcond = &sclkblockcond;
  (allargs + j)->waitingcond = &swaitingcond;
  (allargs + j)->q = q;
  (allargs + j)->seed = rand();

  if(j == nserver + nclient){
    //calling clk function
    clk(allargs + j);
    printf("j is  %d \n",  j);
  } else if (j < nclient) {
    //printf("my seed is %d\n", (allargs + j)->seed);
    //printf("client memory %p at %d \n",  alltids+j, j);
    pthrerr = pthread_create(alltids+j, NULL, &client, (void*) (allargs + j));
    if (pthrerr!=0)
      fatalerr("Main",pthrerr,"Failed to create client thread 1\n");
  } else {
    //printf("server memory %p at %d \n",  alltids+j, j);
    pthrerr = pthread_create(alltids+j, NULL, &server, (void*) (allargs + j));
    if (pthrerr!=0)
      fatalerr("Main",pthrerr,"Failed to create server thread 1\n");
    }
  }

  exit(-1);
}
