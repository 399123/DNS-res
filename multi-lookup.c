#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "util.h"
#include "lookup.c"
#include "queue.c"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"


int main(int argc, char* argv[]){

	/* Local Vars */
    FILE* inputfp = NULL;
    FILE* outputfp = NULL;
    char hostname[SBUFSIZE];
    char errorstr[SBUFSIZE];
    char firstipstr[INET6_ADDRSTRLEN];
    int i;
    
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

	int queue_suc = queue_init(lookup);
	if(!!queue_suc){
		sprintf(errorstr, "Error forming queue");
		return EXIT_FAILURE;
	}
	pthread_t req0;
	pthread_t req1;
	pthread_t req2;
	pthread_t req3;
	pthread_t req4;
	pthread_t req5;
	pthread_t req6;
	pthread_t req7;
	pthread_t req8;
	pthread_t req9;
	pthread_t res1;
	pthread_t res2;
	pthread_t res3;
	pthread_t res4;
	pthread_t res5;
	pthread_t res6;
	pthread_t res7;
	pthread_t res8;
	pthread_t res9;
	pthread_t res0;
	pthread_mutex_t queue_lock;
	pthread_create(req0, NULL, extract(), argv[0]);
	pthread_create(req1, NULL, extract(), argv[1]);
	if(3 != argc){
		pthread_create(req2, NULL, extract(), argv[2]);
	}
	if(4 != argc && argc > 4){
		pthread_create(req3, NULL, extract(), argv[3]);
	}
	if(5 != argc && argc > 5){
		pthread_create(req4, NULL, extract(), argv[4]);
	}
	if(6 != argc && argc > 6){
		pthread_create(req5, NULL, extract(), argv[5]);
	}
	if(1 != argc && argc > 7){
		pthread_create(req6, NULL, extract(), argv[6]);
	}
	if(1 != argc && argc > 8){
		pthread_create(req7, NULL, extract(), argv[7]);
	}
	if(9 != argc && 9 < argc){
		pthread_create(req9, NULL, extract(), argv[9]);
	}
	if(argc != 8 && argc > 8){
		pthread_create(req8, NULL, extract(), argv[8])
	}
}


void extract(const FILE* file){
	char* line[1025];
	int linenumber = 0;
	while(fgets(line, sizeof(line), file) != NULL){
		//if queue is full, sleep for random number mod 100 converted to microseconds
		//else aquire queue push character arrays into the queue
		if(queue_full(lookup)){
			sleep((rand() % 100)/1000000);
		}else{
		queue_push(lookup, line)
		}
	}
	fclose(file);
}

int resolve(FILE* file){
	char* hostname;
	char* ipstring;
	while(!queue_empty(lookup)){
		hostname = queue_pop(lookup);
		&hostname[sizeof(&hostname)/sizeof(&hostname[0])] = ',';
		&hostname[sizeof(&hostname)/sizeof(&hostname[0])] = ' ';
		dnslookup(hostname, ipstring, 1025)
		fwrite(ipstring, sizeof(&ipstring[0]), sizeof(ipstring)/sizeof(&ipstring[0]), file);
	}
}