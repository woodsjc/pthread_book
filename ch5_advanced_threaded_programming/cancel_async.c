#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>


#define SIZE 10
static int matrixa[SIZE][SIZE];
static int matrixb[SIZE][SIZE];
static int matrixc[SIZE][SIZE];



void confirm (int status, char *msg) 
{
	if (status != 0) {
		printf ("%s\n", &msg);
		exit (1);
	}
}


void *thread_routine (void *arg) 
{
	int cancel_type, status;
	int i, j, k, value=1;

	for (i=0; i<SIZE; i++) {
		for (j=0; j<SIZE; j++) {
			matrixa[i][j] = i;
			matrixb[i][j] = j;
		}
	}

	while (1) {
		status = pthread_setcanceltype (
				PTHREAD_CANCEL_ASYNCHRONOUS, &cancel_type);
		confirm (status, "Unable to set thread cancel type.");

		for (i=0; i<SIZE; i++) {
			for (j=0; j<SIZE; j++) {
				matrixc[i][j] = 0;

				for (k=0; k<SIZE; k++) {
					matrixc[i][j] += matrixa[i][k] * matrixb[k][j];
				}
			}
		}

		status = pthread_setcanceltype (cancel_type, &cancel_type);
		confirm (status, "Unable to set thread cancel type back.");

		for (i=0; i<SIZE; i++) {
			for (j=0; j<SIZE; j++) {
				matrixa[i][j] = matrixc[i][j];
			}
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
		printf ("Thread cancelled.\n");
	else
		printf ("Thread was not cancelled.\n");

	return 0;
}

