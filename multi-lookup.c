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
}


extract(FILE* file){
	
}