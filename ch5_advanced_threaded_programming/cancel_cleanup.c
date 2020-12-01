#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>


#define THREADS 5

typedef struct control_tag {
	int counter, busy;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} control_t;

control_t control = {0, 1, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER};


void confirm (int status, char *msg) 
{
	if (status != 0) {
		printf ("%s\n", &msg);
		exit (1);
	}
}


void *cleanup_handler (void *arg) 
{
	control_t *st = (control_t *)arg;
	int status;

	st->counter--;
	printf ("cleanup_handler: counter == %d\n", st->counter);
	status = pthread_mutex_unlock (&st->mutex);
	confirm (status, "Unable to unlock mutex.");
}


void *thread_routine (void *arg) 
{
	int status;

	pthread_cleanup_push (cleanup_handler, (void*)&control);

	status = pthread_mutex_lock (&control.mutex);
	confirm (status, "Unable to lock mutex.");
	control.counter++;

	while (control.busy) {
		status = pthread_cond_wait (&control.cond, &control.mutex);
		confirm (status, "Unable to wait for confrol condition.");
	}

	pthread_cleanup_pop (1);
	return NULL;
}


int main (int argc, char **argv)
{
	pthread_t thread_id[THREADS];
	int count;
	void *result;
	int status;

	for (count=0; count<THREADS; count++) {
		status = pthread_create (
				&thread_id[count], NULL, thread_routine, NULL);
		confirm (status, "Unable to create thread.");
	}

	sleep (2);

	for (count=0; count<THREADS; count++) {
		status = pthread_cancel (thread_id[count]);
		confirm (status, "Unable to cancel thread.");

		status = pthread_join (thread_id[count], &result);
		confirm (status, "Unable to join thread.");

		if (result == PTHREAD_CANCELED) 
			printf ("Thread %d cancelled.\n", count);
		else
			printf ("Thread %d was not cancelled.\n", count);
	}

	return 0;
}

