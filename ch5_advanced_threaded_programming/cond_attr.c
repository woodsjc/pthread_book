#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>


pthread_cond_t cond;


void confirm (int status, char *msg)
{
	if (status != 0) {
		printf ("%s\n", &msg);
		exit (1);
	}
}


int main (int argc, char **argv)
{
	pthread_condattr_t cond_attr;
	int status;

	status = pthread_condattr_init (&cond_attr);
	confirm (status, "Unable to create cond attribute.");

#ifdef _POSIX_THREAD_PROCESS_SHARED	
	status = pthread_mutexattr_setpshared (
			&cond_attr, PTHREAD_PROCESS_PRIVATE);
	confirm (status, "Unable to set pshared.");
#endif

	status = pthread_cond_init (&cond, &cond_attr);
	confirm (status, "Unable to initialize cond.");

	return 0;
}

