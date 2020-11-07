#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>


pthread_mutex_t mutex;


void confirm (int status, char *msg)
{
	if (status != 0) {
		printf ("%s\n", &msg);
		exit (1);
	}
}


int main (int argc, char **argv)
{
	pthread_mutexattr_t mutex_attr;
	int status;

	status = pthread_mutexattr_init (&mutex_attr);
	confirm (status, "Unable to create mutex attribute.");

#ifdef _POSIX_THREAD_PROCESS_SHARED	
	status = pthread_mutexattr_setpshared (
			&mutex_attr, PTHREAD_PROCESS_PRIVATE);
	confirm (status, "Unable to set pshared.");
#endif

	status = pthread_mutex_init (&mutex, &mutex_attr);
	confirm (status, "Unable to initialize mutex.");

	return 0;
}

