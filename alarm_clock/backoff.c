#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>


#define ITERATIONS 10


pthread_mutex_t mutex[3] = {
				PTHREAD_MUTEX_INITIALIZER,
				PTHREAD_MUTEX_INITIALIZER,
				PTHREAD_MUTEX_INITIALIZER
				};

int backoff = 1;
int yield_flag = 0;


void *lock_forward (void *arg)
{
	int i, iterate, backoffs;
	int status;

	for (iterate=0; iterate < ITERATIONS; iterate++) {
		backoffs = 0;
		for (i=0; i < 3; i++) {
			if (i == 0) {
				status = pthread_mutex_lock (&mutex[i]);
				if (status != 0) {
					printf ("Error locking mutex on %d.", i);
					exit (1);
				} 
			} else {
				if (backoff) {
					status = pthread_mutex_trylock (&mutex[i]);
				} else {
					status = pthread_mutex_lock (&mutex[i]);
				}

				if (status == EBUSY) {
					backoffs ++;
					printf ("DEBUG::: forward locker backing off at %d\n", i);
					for (i--; i >= 0; i--) {
						status = pthread_mutex_unlock (&mutex[i]);
						if (status != 0) {
							printf ("Unable to unlock mutex [%d].\n", i);
							exit (1);
						} 
					}
				} else {
					if (status != 0) {
						printf ("DEBUG::: Mutex error backoffs:%d, i:%d.\n", backoffs, i);
						exit (1);
					}
					printf ("DEBUG::: forward locker got %d\n", i);
				}
			}

			if (yield_flag) {
				if (yield_flag > 0) {
					sched_yield ();
				} else {
					sleep (1);
				}
			}
		}

		printf ("Lock forward got all locks, %d backoffs\n", backoffs);
		pthread_mutex_unlock (&mutex[2]);
		pthread_mutex_unlock (&mutex[1]);
		pthread_mutex_unlock (&mutex[0]);
		sched_yield ();
	}
			
	return NULL;
}


void *lock_backward (void *arg)
{
	int i, iterate, backoffs;
	int status;

	for (iterate=0; iterate < ITERATIONS; iterate++) {
		backoffs = 0;

		for (i=2; i >= 0; i--) {
			if (i == 2) {
				status = pthread_mutex_lock (&mutex[i]);
				if (status != 0) {
					printf ("Unable to lock mutex %d.\n", i);
					exit (1);
				}
			} else {
				if (backoff) {
					status = pthread_mutex_trylock (&mutex[i]);
				} else {
					status = pthread_mutex_lock (&mutex[i]);
				}

				if (status == EBUSY) {
					backoffs ++;
					printf ("DEBUG::: backward locker backing off at %d\n", i);

					for (i=0; i < 3; i++) {
						status = pthread_mutex_unlock (&mutex[i]);
						if (status != 0) {
							printf ("Unable to unlock mutex[%d].\n", i);
							exit (1);
						}
					}
				} else {
					if (status != 0) {
						printf ("Unable to lock mutex[%d].\n", i);
						exit (1);
					}
					printf ("DEBUG::: backward locker got %d.\n", i);
				}
			}

			if (yield_flag) {
				if (yield_flag > 0) {
					sched_yield ();
				} else {
					sleep (1);
				}
			}
		}

		printf ("lock backward locker got all locks, %d backoffs\n", backoffs);
		pthread_mutex_unlock (&mutex[0]);
		pthread_mutex_unlock (&mutex[1]);
		pthread_mutex_unlock (&mutex[2]);
		sched_yield ();
	}
		
	return NULL;
}


int main (int argc, char **argv)
{
	pthread_t forward, backward;
	int status;

	if (argc > 1)
		backoff = atoi (argv[1]);

	if (argc > 2)
		yield_flag = atoi (argv[2]);

	status = pthread_create (&forward, NULL, lock_forward, NULL);
	if (status != 0) {
		printf ("Unable to create forward thread.\n");
		exit (1);
	}

	status = pthread_create (&backward, NULL, lock_backward, NULL);
	if (status != 0) {
		printf ("Unable to create backward thread.\n");
		exit (1);
	}

	pthread_exit (NULL);
	return 0;
}
