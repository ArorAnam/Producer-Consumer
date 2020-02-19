//NAMAN ARORA,  19323369
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "cond.c"

// We want threads to be "working" for less than a second
int work (int ms, struct timespec *ts) {
    ts->tv_sec = 0;
    ts->tv_nsec = 1000 * ms;
    nanosleep(ts,NULL);
    return 0;
}
// timespecs for producer and consumer
struct timespec tsP,tsC;

int pnum;  // number updated when producer runs.
int csum;  // sum computed using pnum when consumer runs.

typedef struct {
  char buf[1];
  int bufSize;
  int input;  //incoming value to buffer
  int output;
  pthread_mutex_t mutex;
  pthread_cond_t condK; //conditional variables, mutex
  pthread_cond_t condJ;
} buffer_t;
 
 
buffer_t buffer;

int (*pred)(int); // predicate indicating number to be consumed

int produceT(buffer_t *b) {
  pthread_mutex_lock (&b->mutex);  //acquire lock

  while(b->bufSize >= 1) {
    pthread_cond_wait(&b->condJ, &b->mutex);
    //wait if buufer size if greater than what is availbale
    //until decreased by consumer
  }

  //   CRITICAL_SECTION
  scanf("%d",&pnum); // read a number from stdin
  assert(b->bufSize < 1);  //if the value in b->bufSize condK than 1 throw an error
  b->buf[b->input++] = pnum; 
  //place the value in the buffer if there are no errors

  b->input = b->input % 1;
  b->bufSize++; //increrase the buffer array
  //  END OF CRITICAL SECTION

  pthread_cond_signal(&b->condK);  //passs the control to consumer

  pthread_mutex_unlock (&b->mutex);
  return pnum;
}

void *Produce(void *a) {
  int p;

  p=1;
  while (p) {
    printf("@P-WORK\n");
    work(100,&tsP); // "work" for 100ms
    printf("@P-READY\n");

    p = produceT(&buffer);
    printf("@PRODUCED %d\n",p);
  }
  printf("@P-EXIT\n");
  pthread_mutex_destroy(&buffer.mutex);
  pthread_cond_destroy(&buffer.condK);
  pthread_cond_destroy(&buffer.condJ);
  pthread_exit(NULL);
}


int consumeT(buffer_t *b) {
  pthread_mutex_lock (&b->mutex);  //acquire lock

  while (b->bufSize <= 0) {
    pthread_cond_wait(&b->condK, &b->mutex);
    //wait until the buffer is empty
    //or signal from the producer 
  }

  //   CRTITICAL SECTION
  if( pred(pnum) ) {
    csum += pnum;  //add to total sum
  }

  assert(b->bufSize > 0);
  pnum = b->buf[b->output++];   //make pnum equal to the buffer value corresponding to the output
  b->output %= 1;
  b->bufSize--;
  //  END OF CRITICAL SECTION

  pthread_cond_signal(&b->condJ); // pass control back to the producer thread

  pthread_mutex_unlock (&b->mutex);
  return pnum;
}

void *Consume(void *a) {
  int p;

  p=1;
  while (p) {
    printf("@C-WORK\n");
    work(50+100*(rand()%3),&tsC); // "work" for 50ms, 150ms, or 250ms
    printf("@C-READY\n");

    p = consumeT(&buffer);
    printf("@CONSUMED %d\n",csum);
  }
  printf("@C-EXIT\n");
  pthread_mutex_destroy(&buffer.mutex);
  pthread_cond_destroy(&buffer.condK);
  pthread_cond_destroy(&buffer.condJ);
  pthread_exit(NULL);
}


int main (int argc, const char * argv[]) {
  // the current number predicate
  static pthread_t prod,cons;
	long rc;


  //INITIALISE all values of struct buffer
  buffer.bufSize = 0;
  buffer.input = 0;
  buffer.output = 0;
  pthread_mutex_init(&buffer.mutex, NULL);
  pthread_cond_init(&buffer.condK, NULL);
  pthread_cond_init(&buffer.condJ, NULL);

  pred = &cond1;
  if (argc>1) {
    if      (!strncmp(argv[1],"2",10)) { pred = &cond2; }
    else if (!strncmp(argv[1],"3",10)) { pred = &cond3; }
  }

  pnum = 999;
  csum=0;
  srand(time(0));

  printf("@P-CREATE\n");
 	rc = pthread_create(&prod,NULL,Produce,(void *)0);
	if (rc) {
			printf("@P-ERROR %ld\n",rc);
			exit(-1);
		}
  printf("@C-CREATE\n");
 	rc = pthread_create(&cons,NULL,Consume,(void *)0);
	if (rc) {
			printf("@C-ERROR %ld\n",rc);
			exit(-1);
		}

  printf("@P-JOIN\n");
  pthread_join( prod, NULL);
  printf("@C-JOIN\n");
  pthread_join( cons, NULL);


  printf("@CSUM=%d.\n",csum);

  return 0;
}
