#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "util.h"
#include "queue.c"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"
#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MIN_RESOLVER_THREADS 2
#define MAX_NAME_LENGTH 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN
#define NUMRESOLVE 7

pthread_mutex_t queue_lock;
pthread_mutex_t output_lock;
pthread_mutex_t mutex;
pthread_cond_t empty,full;
int requestercounter;
queue lookup;

int main(int argc, char* argv[]){

	/* Local Vars */
    FILE* inputfp = NULL;
    FILE* outputfp = NULL;
    char hostname[SBUFSIZE];
    char errorstr[SBUFSIZE];
    char firstipstr[INET6_ADDRSTRLEN];
    int i;
    int rc;
    int t;

    //number of requester threads
    int NUM_THREADS = argc - 2;

    //find number of cores for resolver threads
    int NUM_RESOLVE = sysconf(_SC_NPROCESSORS_ONLN) * 4;

    //set counter for request threads
    requestercounter = NUM_THREADS;

    pthread_t requestthreads[NUM_THREADS];
    pthread_t resovethreads[MAX_RESOLVER_THREADS];

    //initialize condition variables and mutex
    pthread_mutex_init(&queue_lock, NULL);
    pthread_mutex_init(&output_lock, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&empty, NULL);
    pthread_cond_init(&full, NULL)

    //initialize queue to -1 so it will run to max queue size
    queue_init(&lookup, -1);

    /* Check Arguments */
    if(argc < MINARGS){
		fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
		return EXIT_FAILURE;
	}

	    /* Open Output File */
	    outputfp = fopen(argv[(argc-1)], "w");
	if(!outputfp){
		perror("Error Opening Output File");
		return EXIT_FAILURE;
    }

	    /* Loop Through Input Files */
	for(i=1; i<(argc-1); i++){
		
		/* Open Input File */
		inputfp = fopen(argv[i], "r");
		if(!inputfp){
		    sprintf(errorstr, "Error Opening Input File: %s", argv[i]);
		    perror(errorstr);
		    break;
		}
	}
		//create requester threads
	for(t = 0; t < NUM_RESOLVE; t++){
		printf("In main: creating requester thread %i\n", t);
		rc = pthread_create(&(requestthreads[t]), NULL, extract, argv[t+1]);
		if(rc){
			printf("Error: return from pthread_create() is %d\n", rc);
			exit(EXIT_FAILURE);
		}
	}

		//create resolver threads == NUM_THREADS
	for(t = 0; t < NUM_RESOLVE; t++){
		printf("In main: creating resolver thread %i\n", t);
		rc = pthread_create(&(resolvethreads[t]), NULL, resolve, NULL);
		if(rc){
			printf("Error: return from pthread_create is %d\n", -1);
			exit(EXIT_FAILURE);
		}
	}
	//wait for requester and resolver threads to finish
	for (t = 0; t < NUM_THREADS; ++t)
	{
		pthread_join(resolvertthreads[t], NULL);
	}
	printf("All resolver threads were completed\n")
	fclose(outputfp);
	queue_cleanup(&lookup);
	return 0;
}	


void* extract(void* inputfp){
	char* line[1025];
	char* DNSstring;
	int linenumber = 0;
	while(fscanf(inputfp, INPUTFS, hostname){
		//if queue is full, sleep for random number mod 100 converted to microseconds
		//else aquire queue push character arrays into the queue
		pthread_mutex_lock(&queue_lock)
		while(queue_is_full(&lookup)){
				pthread_cond_wait(&empty, &queue_lock);
		}
		//allocate memory to store hostname string
		DNSstring = (char*)malloc(sizeof(char)*SBUFSIZE);
		//load hostname into address of where DNSstring is stored
		strcpy(DNSstring, hostname);
		//push address into queu and signal/unlock
		queue_push(&lookup, DNSstring);
		pthread_cond_signal(&full);
		pthread_mutex_unlock(&queue_lock)
	}
	fclose(inputfp);
	//decrement requester thread count
	pthread_mutex_lock(&mutex);
	requestercounter--;
	pthread_mutex_unlock(&mutex);

	return NULL
}

void* resolve(FILE* file){
	char* hostname;
	char ipstr[INET6_ADDRSTRLEN]; 
	//check all requester threads are done and queue is empty
	while(requestercount > 0 || !(queue_is_empty(&lookup))){
		pthread_mutex_lock(&queue_lock);
		//if queue is empty and requester threads remaining wait for nonempty
		//else break loop
		while(queue_is_empty(&lookup)){
			if(requestercount > 0){
				pthread_cond_wait(&empty, &queue_lock);
			}else{
				break;
			}
		}

		//assign hostname to first element in queue
		hostname = queue_pop(&lookup);
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&queue_lock);

		//lookup hostname and get ip address
		//write empty string if error
		if (dnslookup((char*)hostname, ipstr, sizeof(ipstr)) == -1)
		{
			fprintf(stderr, "dnslooup error: %s\n", (char*)hostname);
			stnrcpy(ipstr, "", sizeof(ipstr));
		}

		//write to output
		pthread_mutex_lock(&output_lock)
		fprintf(outputfp, "%s, %S\n", (char*)hostname, ipstr);
		pthread_mutex_unlock(&output_lock);
		//release memory once written
		free(hostname);
	}
	return NULL;
};