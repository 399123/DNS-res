#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "util.h"
#include "queue.h"
#include "multi-lookup.h"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025 
#define INPUTFS "%1024s"
#define QUEUESIZE 50

#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MIN_RESOLVER_THREADS 2
#define MAX_NAME_LENGTH 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN
#define NUMRESOLVE 7

queue QUEUE;
int requestercount;
pthread_cond_t empty,full;
pthread_mutex_t qlock;
pthread_mutex_t olock;
pthread_mutex_t mutex;
FILE* outputfp = NULL;


// requester thread to read input
void* requesterThreads(void* argv){
    // create buffers and pointers
    char hostname[SBUFSIZE];
    char* DNSstring;
    char errorstr[SBUFSIZE];
    FILE* inputfp = NULL;

    // cast inputfp as char pointer to read names
    // open input file print if error
    inputfp = fopen((char*)argv, "r");
    if(!inputfp){
        sprintf(errorstr, "Error Opening Input File: %s", (char*)argv);
        perror(errorstr);
    }

    // read file and load into hostname
    while(fscanf(inputfp, INPUTFS, hostname) > 0){
        pthread_mutex_lock(&qlock);
        while(queue_is_full(&QUEUE)){
            pthread_cond_wait(&empty, &qlock);
        }
    

        // allocate memory to store hostname string
        DNSstring = (char*)malloc(sizeof(char)*SBUFSIZE);

        // load hostname into address of where DNSstring is pointing too
        strcpy(DNSstring, hostname);

        // push address into QUEUE and signal/unlock
        queue_push(&QUEUE, DNSstring);
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&qlock);
    }

    // close input file
    fclose(inputfp);

    // decrement requester thread count
    pthread_mutex_lock(&mutex);
    requestercount--;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

// resolver thread to determine ip address and write into output file
void* resolverThreads(void* arg){
    (void) arg;
    // make string big enough to handle IPv6
    char ipstr[INET6_ADDRSTRLEN];
    void* hostname;

    // base case check for requesters all done and queue empty
    while(requestercount > 0 || !(queue_is_empty(&QUEUE))){
        pthread_mutex_lock(&qlock);
        while(queue_is_empty(&QUEUE)){
            if(requestercount > 0)
                pthread_cond_wait(&empty, &qlock);
            else
                break;
        }


        hostname = queue_pop(&QUEUE);
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&qlock);

        // lookup hostname and get ip address
        // write empty string if error
        if(dnslookup((char*)hostname, ipstr, sizeof(ipstr)) == UTIL_FAILURE){
            fprintf(stderr, "dnslookup error: %s\n", (char*)hostname);
            strncpy(ipstr, "", sizeof(ipstr));
        }

        // write to output
        pthread_mutex_lock(&olock);
        fprintf(outputfp, "%s, %s\n", (char*)hostname, ipstr);
        pthread_mutex_unlock(&olock);
        // release memory once written
        free(hostname);
    }
    return NULL;
}

int main(int argc, char *argv[]){

    // void unused vars
    (void) argc;
    (void) argv;
    int rc;
    int t;

    // number of requester threads
    int NUM_THREADS = argc - 2;

    // find number of cores for resolver threads
    int NUM_RESOLVE = sysconf(_SC_NPROCESSORS_ONLN);

    // set counter for request threads
    requestercount = NUM_THREADS;

    pthread_t requestthreads[NUM_THREADS];
    pthread_t resolvethreads[NUM_RESOLVE];

    // initialize condition variables and mutex
    pthread_mutex_init(&qlock, NULL);
    pthread_mutex_init(&olock, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&empty, NULL);
    pthread_cond_init(&full, NULL);

    // initialize queue to -1 so it will run to max queue size
    queue_init(&QUEUE, -1);

    // open output file, report on error
    outputfp = fopen(argv[argc-1], "a");
    if(!outputfp){
        perror("Error opening output file\n");
        return EXIT_FAILURE;
    }

    // create requester threads == NUM_THREADS
    for(t=0; t<NUM_THREADS; t++){
        printf("In main: creating requester thread %i\n", t);
        rc = pthread_create(&(requestthreads[t]), NULL, requesterThreads, argv[t + 1]);
        if (rc){
            printf("ERROR: return from pthread_create() is %d\n", rc);
            exit(EXIT_FAILURE);
        }
    }

    // create resolver threads == NUM_THREADS
    for(t=0;t<NUM_RESOLVE; t++){
        printf("In main: creating resolver thread %i\n", t);
        rc = pthread_create(&(resolvethreads[t]), NULL, resolverThreads, NULL);
        if (rc){
            printf("ERROR: return from pthread_create() is %d\n", rc);
            exit(EXIT_FAILURE);
        }
    }

    // wait for requester and resolver threads to finish
    for(t=0;t<NUM_THREADS;t++){
        pthread_join(requestthreads[t], NULL);
    }
    printf("All requester threads were compeleted\n");

    //for resolver
    for(t=0;t<NUM_RESOLVE;t++){
        pthread_join(resolvethreads[t], NULL);
    }
    printf("All resolverthreads were completed\n");

    // close output file and clear queue
    fclose(outputfp);
    queue_cleanup(&QUEUE);
    return 0;
}