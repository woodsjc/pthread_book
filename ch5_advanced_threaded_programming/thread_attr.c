#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>


void confirm (int status, char *msg)
{
	if (status != 0) {
		printf ("%s\n", &msg);
		exit (1);
	}
}


void *thread_routine (void *arg)
{
	printf ("The thread is here.\n");
	return NULL;
}


int main (int argc, char **argv)
{
	pthread_t thread_id;
	pthread_attr_t thread_attr;
	struct sched_param thread_param;
	size_t stack_size;
	int status;

	status = pthread_attr_setdetachstate (
			&thread_attr, PTHREAD_CREATE_DETACHED);
	confirm (status, "Unable to set thread attribute detached.");

#ifdef _POSIX_THREAD_ATTR_STACKSIZE	
	status = pthread_attr_getstacksize (&thread_attr, &stack_size);
	confirm (status, "Unable to get stack size.");
	printf ("Default stack size is %u; minimum is %u.\n",
			stack_size, PTHREAD_STACK_MIN);

	status = pthread_attr_setstacksize (
			&thread_attr, PTHREAD_STACK_MIN*2);
	confirm (status, "Unable to set thread attribute stack size.");
#endif

	status = pthread_create (
			&thread_id, &thread_attr, thread_routine, NULL);
	confirm (status, "Unable to create thread.");
	printf ("Main exiting.\n");
	pthread_exit (NULL);

	return 0;
}

