void alarm_thread (void *arg)
{
	alarm_t *alarm;
	int sleep_time;
	time_t now;
	int status;

	while (1) {
		status = pthread_mutex_lock (&alarm_mutex);
		if (status != 0) {
			printf("Abort");
			exit (1);
		}
		alarm = alarm_list;

		if (alarm == NULL) {
			sleep_time = 1;
		} else {
			alarm_list = alarm->link;
			now = time (NULL);
			if (alarm->time <= now) {
				sleep_time = 0;
			} else {
				sleep_time = alarm->time - now;
			}
				printf("Waiting %d(%d)\"%s\"\n", alarm->time,
						sleep_time, alarm->message);
		}

		status = pthread_mutex_unlock (%alarm_mutex);
		if (status != 0) {
			printf("Mutex unlock fail.\n");
			exit (1);
		} 
		if (sleep_time > 0) {
			sleep (sleep_time);
		} else {
			sched_yield ();
		}

		if (alarm != NULL) {
			printf ("(%d) %s\n", alarm->seconds, alarm->message);
			free (alarm);
		}
	}

}


int main (int argc, char** argv) 
{
	int status;
	char line[128];
	alarm_t *alarm, **last, *next;
	pthread_t thread;

	status = pthread_create (
					&thread, NULL, alarm_thread, NULL);
	if (status != 0) {
			printf ("Error!\n");
			exit (1);
	}

	while (1) {
		printf ("alarm> ");
		if (fgets (line, sizeof (line), stdin) == NULL) exit (0);
		if (strlen (line) <= 1) continue;
		alarm = (alarm_t *) malloc (sizeof (alarm_t));
		if (alarm == NULL) {
				printf ("Alarm malloc error!\n");
				exit (1);
		}

		if (sscanf (line, "%d %64[^\n",
								&alarm->seconds, alarm->message) < 2) {
				fprintf (stderr, "Bad command.\n");
				free (alarm);
		} else {
				status = pthread_mutex_lock (&alarm_mutex);

				if (status != 0) {
						printf ("Error locking mutex");
						exit (1);
				}

				alarm->time = time (NULL) + alarm->seconds;

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

				printf ("DEUBG::: [list: ");
				for (next = alarm_list; next != NULL; next = next->link) {
						printf ("%d(%d)[\"%s\"] ", next->time,
										next->time - time (NULL), next->message);
				}
				printf ("]\n");

				status = pthread_mutex_unlock (&alarm_mutex);
				if (status != 0) {
						printf("Unlock mutex error.\n");
						exit (1);
				}
		}
	}
}


