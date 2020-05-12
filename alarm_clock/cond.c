#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>


typedef struct cond_mutex {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int value;
} cond_mutex;

cond_mutex data = {
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_COND_INITIALIZER,
	0
};

int hibernation = 1;


void *wait_thread (void *arg)
{
	int status;

	sleep (hibernation);
	status = pthread_mutex_lock (&data.mutex);
	if (status != 0) {
		printf ("Unable to lock mutex on data.mutex:%p.\n", &data.mutex);
		exit (1);
	}

	data.value = 1;
	status = pthread_cond_signal (&data.cond);
	if (status != 0) {
		printf ("Unable to signal condition on data.cond:%p.\n", &data.cond);
		exit (1);
	}

	status = pthread_mutex_unlock (&data.mutex);
	if (status != 0) {
		printf ("Unable to unlock data.mutex:%p.\n", &data.mutex);
		exit (1);
	}

	return NULL;
}


int main (int argc, char **argv)
{
	int status;
	pthread_t wait_thread_id;
	struct timespec timeout;

	if (argc > 1)
		hibernation = atoi (argv[1]);

	status = pthread_create (&wait_thread_id, NULL, wait_thread, NULL);
	if (status != 0) {
		printf ("Unable to create wait thread.\n");
		exit (1);
	}

	timeout.tv_sec = time (NULL) + 2;
	timeout.tv_nsec = 0;
	status = pthread_mutex_lock (&data.mutex);
	if (status != 0) {
		printf ("Unable to lock data.mutex:%p.\n", &data.mutex);
		exit (1);
	}

	while (data.value == 0) {
		status = pthread_cond_timedwait (&data.cond, &data.mutex, &timeout);
		if (status == ETIMEDOUT) {
			printf ("Condition wait timed out.\n");
			break;
		} else if (status != 0) {
			printf ("Unable to access data.cond:%p.\n", &data.cond);
			exit (1);
		}
	}

	if (data.value != 0) {
		printf ("Condition was signaled!\n");
	}
	status = pthread_mutex_unlock (&data.mutex);
	if (status != 0) {
		printf ("Unable to unlock data.mutex:%p.\n", &data.mutex);
		exit (1);
	}

	return 0;
}


