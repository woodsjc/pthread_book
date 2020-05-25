#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>


# define CREW_SIZE 4

typedef struct work_tag {
	struct work_tag *next;
	char *path;
	char *string;
} work_t, *work_p;

typedef struct worker_tag {
	int index;
	pthread_t thread;
	struct crew_tag *crew;
} worker_t, *worker_p;

typedef struct crew_tag {
	int crew_size;
	worker_t crew[CREW_SIZE];
	long work_count;
	work_t *first, *last;
	pthread_mutex_t mutex;
	pthread_cond_t done, go;
	long path_max, name_max;
} crew_t, *crew_p;


void confirm (int status, char *msg)
{
	if (status != 0) {
		printf ("%s", &msg);
		exit (1);
	}
	return;
}


void *worker_routine (void *arg)
{
	worker_p mine = (worker_t*)arg;
	crew_p crew = mine->crew;
	work_p work, new_work;
	struct stat filestat;
	struct dirent *entry;
	int status;

	status = pthread_mutex_unlock (&crew->mutex);
	confirm (status, "Unable to create lock crew mutex.\n");

	while (crew->work_count == 0) {
		status = pthread_cond_wait (&crew->go, &crew->mutex);
		confirm (status, "Unable to wait on condition.\n");
	}

	status = pthread_mutex_unlock (&crew->mutex);
	confirm (status, "Unable to unlock mutex.\n");

	printf ("Crew %d starting.\n", mine->index);

	entry = (struct dirent*)malloc (sizeof (struct dirent) + crew->name_max);
	if (entry == NULL) {
		printf ("Unable to malloc.\n");
		exit (1);
	}

	while (1) {
		status = pthread_mutex_lock (&crew->mutex);
		confirm (status, "Unable to lock mutex.\n");

		printf ("Crew %d top: first is %#lx, count is %d\n", mine->index, crew->first, crew->work_count);

		while (crew->first == NULL) {
			status = pthread_cond_wait (&crew->go, &crew->mutex);
			confirm (status, "Unable to wait.\n");
		}

		printf ("Crew %d work: %#lx, %d\n", mine->index, crew->first, crew->work_count);

		work = crew->first;
		crew->first = work->next;
		if (crew->first == NULL)
			crew->last = NULL;

		printf ("Crew %d took %#lx, leaves first %#lx, last %#lx\n", mine->index, work, crew->first, crew->last);

		status = pthread_mutex_unlock (&crew->mutex);
		confirm (status, "Unable to unlock mutex.\n");

		status = lstat (work->path, &filestat);

		if (S_ISLNK (filestat.st_mode))
			printf ("Thread %d: %s is a link, skipping.\n", mine->index, work->path);
		else if (S_ISDIR (filestat.st_mode)) {
			DIR *directory;
			struct dirent *result;

			directory = opendir (work->path);
			if (directory == NULL) {
				fprintf (stderr, "Unable to open directory %s: %d (%s).\n",
						work->path, errno, strerror (errno));
				continue;
			}

			while (1) {
				status = readdir_r (directory, entry, &result);
				if (status != 0) {
					fprintf( stderr, "Unable to read directory %s: %d (%s).\n",
							work->path, status, strerror (status));
					break;
				}

				if (result == NULL) 
					break;

				if (strcmp (entry->d_name, ".") == 0)
					continue;

				if (strcmp (entry->d_name, "..") == 0)
					continue;

				new_work =(work_p)malloc (sizeof (work_t));
				if (new_work == NULL) {
					printf ("Unable to allocate memory for new_work.\n");
					exit (1);
				}
				
				new_work->path = (char*)malloc (sizeof (crew->path_max));
				if (new_work->path == NULL) {
					printf ("Unable to allocate memory for new_work->path.\n");
					exit (1);
				}

				strcpy (new_work->path, work->path);
				strcat (new_work->path, "/");
				strcat (new_work->path, entry->d_name);

				new_work->string = work->string;
				new_work->next = NULL;
				
				status = pthread_mutex_lock (&crew->mutex);
				confirm (status, "Unable to lock mutex.\n");

				if (crew->first == NULL) {
					crew->first = new_work;
					crew->last = new_work;
				} else {
					crew->last->next = new_work;
					crew->last = new_work;
				}

				crew->work_count ++;

				printf ("Crew %d: add %#lx, first %#lx, last %#lx, %d.\n",
						mine->index, new_work, crew->first, crew->last, crew->work_count);

				status = pthread_cond_signal (&crew->go);
				status = pthread_mutex_unlock (&crew->mutex);
				confirm (status, "Unable to unlock mutex.\n");
			}

			closedir (directory);
		} else if (S_ISREG (filestat.st_mode)) {
			FILE *search;
			char buffer[256], *bufptr, *search_ptr;

			search = fopen (work->path, "r");
			if (search == NULL) {
				fprintf (stderr, "Unable to open %s: %d (%s).\n",
						work->path, errno, strerror (errno));
			} else {
				while (1) {
					bufptr = fgets (buffer, sizeof (buffer), search);
					if (bufptr == NULL) {
						if (feof (search))
							break;
						if (ferror (search)) {
							fprintf (stderr, "Unable to read %s: %d (%s).\n",
									work->path, errno, strerror (errno));
							break;
						}
					}
					search_ptr = strstr (buffer, work->string);
					if (search_ptr != NULL) {
						printf ("Thread %d found \"%s\" in %s.\n",
								mine->index, work->string, work->path);
						break;
					}
				}
				fclose (search);
			}
		} else {
			fprintf (stderr, "Thread %d: %s is type %o (%s)).\n",
					mine->index, work->path, 
					filestat.st_mode & S_IFMT,
					(S_ISFIFO (filestat.st_mode) ? "FIFO"
					 : (S_ISCHR (filestat.st_mode) ? "CHR"
						 : (S_ISBLK (filestat.st_mode) ? "BLK"
							 : (S_ISSOCK (filestat.st_mode) ? "SOCK"
								 : "unknown")))));
		}

		free (work->path);
		free (work);

		status = pthread_mutex_lock (&crew->mutex);
		confirm (status, "Unable to lock crew mutex.\n");

		crew->work_count --;
		printf ("Crew %d decremented work to %d.\n", mine->index, crew->work_count);

		if (crew->work_count <= 0) {
			printf ("Crew thread %d done.\n", mine->index);
			status = pthread_cond_broadcast (&crew->done);
			confirm (status, "Unable to wake waiting threads.\n");

			status = pthread_mutex_unlock (&crew->mutex);
			confirm (status, "Unable to unlock crew mutex.\n");
		}

		status = pthread_mutex_unlock (&crew->mutex);
		confirm (status, "Unable to unlock crew mutex.\n");
	}
	
	free (entry);
	return NULL;
}


int crew_create (crew_t *crew, int crew_size)
{
	int crew_index;
	int status;

	if (crew_size > CREW_SIZE) 
		return EINVAL;

	crew->crew_size = crew_size;
	crew->work_count = 0;
	crew->first = NULL;
	crew->last = NULL;

	status = pthread_mutex_init (&crew->mutex, NULL);
	confirm (status, "Unable to initialize mutex.\n");

	status = pthread_cond_init (&crew->done, NULL);
	confirm (status, "Unable to initialize condition.\n");

	status = pthread_cond_init (&crew->go, NULL);
	confirm (status, "Unable to initialize condition for crew->go.\n");

	for (crew_index=0; crew_index < CREW_SIZE; crew_index++) {
		crew->crew[crew_index].index = crew_index;
		crew->crew[crew_index].crew = crew;

		status = pthread_create (&crew->crew[crew_index].thread,
				NULL, worker_routine, (void*)&crew->crew[crew_index]);
		confirm (status, "Unable to create worker.\n");
	}

	return 0;
}


int crew_start (crew_p crew, char *filepath, char *search)
{
	work_p request;
	int status;

	status = pthread_mutex_lock (&crew->mutex);
	confirm (status, "Unable to lock mutex.\n");

	while (crew->work_count > 0) {
		status = pthread_cond_wait (&crew->done, &crew->mutex);
		if (status != 0) {
			pthread_mutex_unlock (&crew->mutex);
			return status;
		}
	}

	errno = 0;
	crew->path_max = pathconf (filepath, _PC_PATH_MAX);
	if (crew->path_max == -1) {
		if (errno == 0)
			crew->path_max = 1024;
		else 
			confirm (-1, "Unable to get PATH_MAX.\n");
	}

	errno = 0;
	crew->name_max = pathconf (filepath, _PC_NAME_MAX);
	if (crew->name_max == -1) {
		if (errno == 0)
			crew->name_max = 256;
		else 
			confirm (-1, "Unable to get PATH_MAX.\n");
	}

	printf ("PATH_MAX for %s is %ld, NAME_MAX is %ld.\n",
			filepath, crew->path_max, crew->name_max);
	crew->path_max ++;
	crew->name_max ++;
	request = (work_p)malloc (sizeof (work_t));
	if (request == NULL) {
		printf ("Unable to allocate memory for request.\n");
		exit (1);
	}

	printf ("Requesting %s.\n", filepath);
	request->path = (char*)malloc (crew->path_max);
	if (request->path == NULL) {
		printf ("Unable to allocate memory for request->path.\n");
		exit (1);
	}

	strcpy (request->path, filepath);
	request->string = search;
	request->next = NULL;

	if (crew->first == NULL) {
		crew->first = request;
		crew->last = request;
	} else {
		crew->last->next = request;
		crew->last = request;
	}

	crew->work_count ++;
	status = pthread_cond_signal (&crew->go);
	if (status != 0) {
		free (crew->first);
		crew->first = NULL;
		crew->work_count = 0;
		pthread_mutex_unlock (&crew->mutex);
		return status;
	}

	while (crew->work_count > 0) {
		status = pthread_cond_wait (&crew->done, &crew->mutex);
		confirm (status, "Waiting for crew to finish.\n");
	}

	status = pthread_mutex_unlock (&crew->mutex);
	confirm (status, "Unable to unlock crew->mutex.\n");

	return 0;
}


int main (int argc, char **argv)
{
	crew_t my_crew;
	char line[128], *next;
	int status;

	if (argc < 3) {
		fprintf (stderr, "Usage: %s string path.\n", argv[0]);
		return -1;
	}

	status = crew_create (&my_crew, CREW_SIZE);
	confirm (status, "Unable to create crew.\n");

	status = crew_start (&my_crew, argv[2], argv[1]);
	confirm (status, "Unable to start crew.\n");

	return 0;
}

