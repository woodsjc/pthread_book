#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>


#define THREADS 5

typedef struct team_tag {
	int join_i;
	pthread_t workers[THREADS];
} team_t;


void confirm (int status, char *msg) 
{
	if (status != 0) {
		printf ("%s\n", &msg);
		exit (1);
	}
}


void *worker_routine (void *arg)
{
	int counter;

	for (counter=0; ; counter++) {
		if ((counter % 1000) == 0) 
			pthread_testcancel ();
	}
}


void *cleanup (void *arg) 
{
	team_t *team = (team_t *)arg;
	int count, status;

	for (count=team->join_i; count < THREADS; count++) {
		status = pthread_cancel (team->workers[count]);
		confirm (status, "Unable to cancel worker.");

		status = pthread_detach (team->workers[count]);
		confirm (status, "Unable to detach worker.");
		printf ("cleanup: cancelled %d\n", count);
	}
}


void *thread_routine (void *arg) 
{
	team_t team;
	int count;
	void *result;
	int status;

	for (count=0; count<THREADS; count++) {
		status = pthread_create (
				&team.workers[count], NULL, worker_routine, NULL);
		confirm (status, "Unable to create thread.");
	}

	pthread_cleanup_push (cleanup, (void*)&team);

	for (team.join_i=0; team.join_i<THREADS; team.join_i++) {
		status = pthread_join (team.workers[team.join_i], &result);
		confirm (status, "Unable to join team workers.");
	}

	pthread_cleanup_pop (0);
	return NULL;
}


int main (int argc, char **argv)
{
	pthread_t thread_id;
	int status;

	status = pthread_create (&thread_id, NULL, thread_routine, NULL);
	confirm (status, "Unable to create team.");

	sleep (5);
	printf ("Cancelling...\n");

	status = pthread_cancel (thread_id);
	confirm (status, "Unable to cancel team.");

	status = pthread_join (thread_id, NULL);
	confirm (status, "Unable to join team.");
	
	return 0;
}

