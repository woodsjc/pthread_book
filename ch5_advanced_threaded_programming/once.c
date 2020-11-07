#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>


pthread_once_t once_block = PTHREAD_ONCE_INIT;
pthread_mutex_t mutex;


void confirm (int status, char *msg)
{
	if (status != 0) {
		printf ("%s\n", &msg);
		exit (1);
	}
}


void once_init_routine ()
{
	int status;

	status = pthread_mutex_init (&mutex, NULL);
	confirm (status, "Unable to initialize mutex.");
}


void *thread_routine (void *arg)
{
	int status;

	status = pthread_once (&once_block, once_init_routine);
	confirm (status, "Unable to initialize once.");

	status = pthread_mutex_lock (&mutex);
	confirm (status, "Unable to lock mutex.");

	printf ("thread_routine has locked the mutex.\n");

	status = pthread_mutex_unlock (&mutex);
	confirm (status, "Unable to unlock mutex.");

	return NULL;
}


int main (int argc, char **argv)
{
	pthread_t thread_id;
	char *input, buffer[64];
	int status;

	status = pthread_create (&thread_id, NULL, thread_routine, NULL);
	confirm (status, "Unable to create thread.");

	status = pthread_once (&once_block, once_init_routine);
	confirm (status, "Unable to initialize once routine.");

	status = pthread_mutex_lock (&mutex);
	confirm (status, "Unable to lock mutex.");
	printf ("Main thread has locked mutex.\n");

	status = pthread_mutex_unlock (&mutex);
	confirm (status, "Unable to unlock mutex.");

	status = pthread_join (thread_id, NULL);
	sprintf (buffer, "Unable to join thread %d.\n", thread_id);
	confirm (status, buffer);

	return 0;
}

