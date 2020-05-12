#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>


typedef struct alarm_tag {
	struct alarm_tag *link;
	int seconds;
	time_t time;
	char message[64];
} alarm_t;

pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t alarm_cond = PTHREAD_COND_INITIALIZER;
alarm_t *alarm_list = NULL;
time_t current_alarm = 0;


void alarm_insert (alarm_t *alarm)
{
	int status;
	alarm_t **last, *next;

	last = &alarm_list;
	next = *last;
	while (next != NULL) {
		if (next->time >= alarm->time) {
			alarm->link = next;
			*last = alarm;
			break;
		}
		last = &next->link;
		next = next->link;
	}

	if (next == NULL) {
		*last = alarm;
		alarm->link = NULL;
	}

#ifdef DEBUG
	printf ("[list: ");
	for (next=alarm_list; next != NULL; next=next->link) {
		printf ("%d(%d)[\"%s\"] ", 
				next->time, 
				next->time - time (NULL),
				next->message
				);
	}
	printf ("]\n");
#endif

	if (current_alarm == 0 || alarm->time < current_alarm) {
		current_alarm = alarm->time;
		status = pthread_cond_signal (&alarm_cond);
		if (status != 0) {
			printf ("Unable to signal alarm condition %s.\n", alarm->message);
			exit (1);
		}
	}

	return;
}


void *alarm_thread (void *args)
{
	alarm_t *alarm;
	struct timespec cond_time;
	time_t now;
	int status, expired;

	status = pthread_mutex_lock (&alarm_mutex);
	if (status != 0) {
		printf ("Unable to lock mutex - alarm_mutex:%p.\n", &alarm_mutex);
		exit (1);
	}

	while (1) {
		current_alarm = 0;
		while (alarm_list == NULL) {
			status = pthread_cond_wait (&alarm_cond, &alarm_mutex);
			if (status != 0) {
				printf ("Unable to wait on condition alarm_cond:%p alarm_mutex:%p.\n", 
						&alarm_cond, &alarm_mutex);
				exit (1);
			}
		}

		alarm = alarm_list;
		alarm_list = alarm->link;
		now = time (NULL);
		expired = 0;
	
		if (alarm->time > now) {
#ifdef DEBUG
			printf ("DEBUG::: waiting :%d(%d)'%s'\n", 
					alarm->time,
					alarm->time - time (NULL),
					alarm->message
					);
#endif

			cond_time.tv_sec = alarm->time;
			cond_time.tv_nsec = 0;
			current_alarm = alarm->time;

			while (current_alarm == alarm->time) {
				status = pthread_cond_timedwait (&alarm_cond, &alarm_mutex, &cond_time);
				if (status == ETIMEDOUT) {
					expired = 1;
					break;
				} else if (status != 0) {
					printf ("Unable to get condition within time cond_time:%p.\n", &cond_time);
					exit (1);
				}
			}

			if (!expired) 
				alarm_insert (alarm);

		} else 
			expired = 1;

		if (expired) {
			printf ("(%d) %s\n", alarm->seconds, alarm->message);
			free (alarm);
		}
	}

	return NULL;
}


int main (int argc, char **argv)
{
	int status;
	char line[128];
	alarm_t *alarm;
	pthread_t thread;

	status = pthread_create (&thread, NULL, alarm_thread, NULL);
	if (status != 0) {
		printf ("Unable to create thread:%p.\n", &thread);
		exit (1);
	}

	while (1) {
		printf ("Alarm> ");
		if (fgets (line, sizeof (line), stdin) == NULL) exit (0);
		if (strlen (line) <= 1) continue;

		alarm = (alarm_t*)malloc (sizeof (alarm_t));
		if (alarm == NULL) {
			printf ("Unable to malloc alarm.\n");
			exit (1);
		}

		if (sscanf (line, "%d %64[^\n]", 
					&alarm->seconds, &alarm->message) < 2) {
			fprintf (stderr, "Bad command\n");
			free (alarm);
		} else {
			status = pthread_mutex_lock (&alarm_mutex);
			if (status != 0) {
				printf ("Unable to lock alarm_mutex:%p.\n", &alarm_mutex);
				exit (1);
			}

			alarm->time = time (NULL) + alarm->seconds;
			alarm_insert (alarm);
			status = pthread_mutex_unlock (&alarm_mutex);
			if (status != 0) {
				printf ("Unable to unlock alarm_mutex:%p.\n", &alarm_mutex);
				exit (1);
			}
		}
	}

	return 0;
}
