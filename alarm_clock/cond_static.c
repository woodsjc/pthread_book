#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


typedef struct my_struct_tag {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int value;
} my_struct_t;

my_struct_t data = {
	PTHREAD_MUTEX_INITIALIZER, 
	PTHREAD_COND_INITIALIZER,
	0
};


int main ()
{
	return 0;
}


