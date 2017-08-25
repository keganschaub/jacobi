#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>

#define MAXITERS 3000
#define MAX_GRID 250
#define MAX_THREAD 16
#define SHARED 0

//Global variables

//For the barrier
/*
pthread_mutex_t barrier_mutex;
pthread_cond_t go;
*/

sem_t mutex;
int arrive[MAX_THREAD], gridSize, threadSize, numWorkers, curr_id = 0;
float grid[MAX_GRID][MAX_GRID], new[MAX_GRID][MAX_GRID], diffThreads[MAX_THREAD], epsilon = 0;

//Functions
void *Worker(void *);
void barrier(int id);

int main(int argc, char *argv[]){

    //Variables
    long i, j;
    int result;
    float left = 1.0, top = 1.0, right = 1.0, bottom = 1.0;
    FILE *outputFile;
    // used for timing
    struct timeval startTime, endTime;
    int seconds, micros;
    //Required arguments
    if (argc > 1){
        gridSize = atoi(argv[1]);
        numWorkers = atoi(argv[2]);
    }
    else{
        fprintf(stderr, "Too few arguments\n");
    }

    //Optional arguments
    if (argc > 3){
        left = atof(argv[3]);
    }
    if (argc > 4){
        top = atof(argv[4]);
    }
    if (argc > 5){
        right = atof(argv[5]);
    }
    if (argc > 6){
        bottom = atof(argv[6]);
    }
    if (argc > 7){
        epsilon = atof(argv[7]);
    }

    //Each thread will be this size
    threadSize = gridSize/numWorkers;

    //Initialize pthread
    pthread_t worker_threads[numWorkers];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

/*
    //Initialize mutex/cond
    pthread_mutex_init(&barrier_mutex, NULL);
    pthread_cond_init(&go, NULL);
*/
    //Initialize semaphore
    result = sem_init(&mutex, SHARED, 1);
    if (result != 0){
        fprintf(stderr, "could not init mutex, errno = %d\n", errno);
        perror("mutex init failed");
        exit(1);
    }

    //Initialize arrive and diffThreads
    for (i = 0; i < numWorkers; i++){
        arrive[i] = 0;
        diffThreads[i] = 0.0;
    }

    //Initialize grids
    for (i = 0; i < (gridSize+2); i++){
        for (j = 0; j < (gridSize+2); j++){
            //Top
            if(i == 0){
                grid[i][j] = top;
                new[i][j] = top;
            }
            //Left
            else if (j == 0){
                grid[i][j] = left;
                new[i][j] = left;
            }
            //Bottom
            else if (i == (gridSize+1)){
                grid[i][j] = bottom;
                new[i][j] = bottom;
            }
            //Right
            else if (j == (gridSize+1)){
                grid[i][j] = right;
                new[i][j] = right;
            }
            //Middle
            else{
                grid[i][j] = 0;
                new[i][j] = 0;
            }
        }
    }

    //Start timer here
    gettimeofday(&startTime, NULL);

    //Create and join threads
    for (i = 0; i < numWorkers; i++){
        pthread_create(&worker_threads[i], &attr, Worker, (void *)i);
    }
    for (i = 0; i < numWorkers; i++){
        pthread_join(worker_threads[i], NULL);
    }

    //End timer here
    gettimeofday(&endTime, NULL);

    //Finalize time measurements
    seconds = endTime.tv_sec - startTime.tv_sec;
    micros = endTime.tv_usec - startTime.tv_usec;
    if ( endTime.tv_usec < startTime.tv_usec ) {
        micros += 1000000;
        seconds--;
    }

    //Destroy semaphore
    result = sem_destroy(&mutex);
    if (result != 0){
        fprintf(stderr, "could not destroy mutex, errno = %d\n", errno);
        perror("mutex destroy failed");
        exit(1);
    }

    //Print to stdout
    printf("main: numProcs = %d, N = %d\n", numWorkers, gridSize);
    printf("execution time = %d seconds, %d microseconds\n",
            seconds, micros);

    //Write to file
    outputFile = fopen("JacobiAnsC.txt", "w");

    fprintf(outputFile, "Grid     = %d x %d\n", gridSize, gridSize);
    fprintf(outputFile, "numProcs = %d\n", numWorkers);
    fprintf(outputFile, "left     =%8.4f\n", left);
    fprintf(outputFile, "top      =%8.4f\n", top);
    fprintf(outputFile, "right    =%8.4f\n", right);
    fprintf(outputFile, "bottom   =%8.4f\n", bottom);
    fprintf(outputFile, "epsilon  =%8.4f\n", epsilon);
    fprintf(outputFile, "execution time = %d seconds, %d microseconds\n\n",
            seconds, micros);
    for (i = 0; i < (gridSize+2); i++){
        for (j = 0; j < (gridSize+2); j++){
            fprintf(outputFile, "%9.4f", grid[i][j]);
        }
        fprintf(outputFile, "\n");
    }

    pthread_exit(NULL);

    return 0;
}

void *Worker(void *arg){
    long id = (long)arg;
    int i, j,  iters = 1, firstRow, lastRow;
    double maxdiff = 0, temp;

    firstRow = id * threadSize +1;
    lastRow = firstRow + threadSize -1;


    barrier(id);
    for (iters = 1; iters <= MAXITERS; iters  += 2){
        for (i = firstRow; i <= lastRow; i++){
            for (j = 1; j <= gridSize; j++){
                new[i][j] = (grid[i-1][j] + grid[i+1][j] + grid[i][j-1] + grid[i][j+1]) * 0.25;
            }
        }
        barrier(id);
        for (i = firstRow; i <= lastRow; i++){
            for (j = 1; j <= gridSize; j++){
                grid[i][j] = (new[i-1][j] + new[i+1][j] + new[i][j-1] + new[i][j+1]) * 0.25;
            }
        }
        barrier(id);
    }

    barrier(id);
    maxdiff = 0.0;
    for (i = firstRow; i <= lastRow; i++){
        for (j = 1; j <= gridSize; j++){
            temp = grid[i][j] - new[i][j];
            if (temp < 0){
                temp = -temp;
            }
            if (temp > maxdiff){
                maxdiff = temp;
            }
        }
    }
    diffThreads[id] = maxdiff;

    pthread_exit( NULL );
}

void barrier(int id){

    int result, stage = 1;

    while(stage < numWorkers){

        arrive[id]++;

        //P(mutex)
        result = sem_wait(&mutex);
        if ( result != 0 ) {
            fprintf(stderr, "mutex: wait failed, errno = %d\n", errno);
            perror("mutex: wait failed");
            exit(1);
        }

        while (! (arrive[ ((id + stage) % numWorkers)] >= arrive[id])){

//            printf("id = %d stage = %d\n", id, stage);

            //V(mutex)
            result = sem_post(&mutex);
            if ( result != 0 ) {
                fprintf(stderr, "mutex: post failed, errno = %d\n", errno);
                perror("mutex: post failed");
                exit(1);
            }

            //Delay
            usleep(1000); 
            
            //P(mutex)
            result = sem_wait(&mutex);
            if ( result != 0 ) {
                fprintf(stderr, "mutex: wait failed, errno = %d\n", errno);
                perror("mutex: wait failed");
                exit(1);
            }

        }

        //V(mutex)
        result = sem_post(&mutex);
        if ( result != 0 ) {
            fprintf(stderr, "mutex: post failed, errno = %d\n", errno);
            perror("mutex: post failed");
            exit(1);
        }

        stage = stage * 2;

    }

    /*
    pthread_mutex_lock(&barrier_mutex);
    curr_id++;
    if (curr_id == numWorkers) {
        curr_id = 0;
        pthread_cond_broadcast(&go);
    } else
        pthread_cond_wait(&go, &barrier_mutex);
    pthread_mutex_unlock(&barrier_mutex);
    */
    
}
