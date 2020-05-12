#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


typedef struct my_struct_tag {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int value;
} my_struct_t;


int main (int argc, char **argv)
{
	my_struct_t *data;
	int status;

	data = malloc (sizeof (my_struct_t));
	if (data == NULL) {
		printf ("Unable to malloc %d.\n", sizeof (my_struct_t));
		exit (1);
	}

	status = pthread_mutex_init (&data->mutex, NULL);
	if (status != 0) {
		printf ("Unable to initialize mutex.\n");
		exit (1);
	}

	status = pthread_cond_init (&data->cond, NULL);
	if (status != 0) {
		printf ("Unable to initialize condition.\n");
		exit (1);
	}

	status = pthread_cond_destroy (&data->cond);
	if (status != 0) {
		printf ("Unable to destroy condition.\n");
		exit (1);
	}

	status = pthread_mutex_destroy (&data->mutex);
	if (status != 0) {
		printf ("Unable to destroy mutex.\n");
		exit (1);
	}

	(void)free (data);
	return status;
}
