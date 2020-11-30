#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>


static int counter;


void confirm (int status, char *msg) 
{
	if (status != 0) {
		printf ("%s\n", &msg);
		exit (1);
	}
}


void *thread_routine (void *arg) 
{
	int state;
	int status;

	for (counter=0; ; counter++) {
		if ((counter % 755) == 0) {
			status = pthread_setcancelstate (
					PTHREAD_CANCEL_DISABLE, &state);
			confirm (status, "Unable to set thread cancel state.");
			sleep (1);

			status = pthread_setcancelstate (state, &state);
			confirm (status, "Unable to set thread cancel state second time.");
		} else {
			if ((counter % 1000) == 0)
				pthread_testcancel ();
		}
	}
}


int main (int argc, char **argv)
{
	pthread_t thread_id;
	void *result;
	int status;

	status = pthread_create (
			&thread_id, NULL, thread_routine, NULL);
	confirm (status, "Unable to create thread.");
	sleep (2);

	printf ("Calling cancel\n");
	status = pthread_cancel (thread_id);
	confirm (status, "Unable to cancel thread.");

	printf ("Calling join\n");
	status = pthread_join (thread_id, &result);
	confirm (status, "Unable to join thread.");

	if (result == PTHREAD_CANCELED) 
		printf ("Thread cancelled at iteration %d\n", counter);
	else
		printf ("Thread was not cancelled.\n");

	return 0;
}

