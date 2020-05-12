#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


void *thread_routine (void *arg)
{
    return arg;
}

void error_abort(int status)
{
        printf("Error %d.\n", status);
        exit (status);
}

int main (int argc, char **argv)
{
    pthread_t thread_id; //unsigned long
    void *thread_result;
    int status;

    status = pthread_create (&thread_id, NULL, thread_routine, NULL);
    if (status != 0) {
        error_abort (status);
    }
    printf("Thread created %lu.\n", thread_id);
    
    status = pthread_join (thread_id, &thread_result);
    if (status != 0)
        error_abort (status);
    if (thread_result == NULL) {
        return 0;
    } else {
        return 1;
    }
    printf("Thread joined %lu.\n", thread_id);

}