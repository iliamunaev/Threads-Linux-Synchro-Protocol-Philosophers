#include "philo.h"

// Function to display usage information
void usage(char *s)
{
	fprintf(stderr, "usage: phil num-philosophers max-think [time_to_eat] [time_to_sleep] accounting-interval seed(-1=time(0))\n");
	if (strlen(s) > 0) fprintf(stderr, "%s\n", s);
	exit(1);
}


int main(int argc, char **argv)
{
	// Check if the number of arguments is correct
	if (argc != 7)
		usage("");

	int *ids;
	pthread_t *tids, atid;
	int i;
	long seed;
	struct timeval tv;

	t_data	d;
	t_data	*data;

	data = &d;


	// PS = &P; // Set pointer to the global philosopher structure
	data->num = atoi(argv[1]); // Get the number of philosophers
	data->maxthink = atoi(argv[2]); // Max thinking time
	data->time_to_eat = atoi(argv[3]); // Max eating time
	data->time_to_sleep = atoi(argv[4]); // Max eating time
	data->accounting_interval = atoi(argv[5]); // Accounting interval
	sscanf(argv[6], "%ld", &seed); // Get the random seed from the command line
	if (seed == -1)
		seed = time(0);  // Default seed if -1

	if (data->num <= 0 || data->maxthink <= 0 || data->time_to_eat <= 0)
	{
		usage("num-philosophers, max-think and max-eat all have to be greater than zero");
	}

	ids = malloc( data->num * sizeof(int));
	if (!ids)
	{
		fprintf(stderr, "Error: Failed to allocate memory for ids.\n");
		exit(1);
	}
	tids = malloc(data->num * sizeof(pthread_t));
	if (!tids)
	{
		fprintf(stderr, "Error: Failed to allocate memory for tids.\n");
		exit(1);
	}
	data->lock = malloc(data->num * sizeof(pthread_mutex_t));
	if (!data->lock)
	{
		fprintf(stderr, "Error: Failed to allocate memory for mutex lock.\n");
		exit(1);
	}
	data->stick_states = malloc(data->num * sizeof(int));
	if (!data->stick_states)
	{
		fprintf(stderr, "Error: Failed to allocate memory for stick_states.\n");
		exit(1);
	}

	data->phil_states = malloc( data->num * sizeof(int));
	if (!data->phil_states)
	{
		fprintf(stderr, "Error: Failed to allocate memory for phil_states.\n");
		exit(1);
	}

	data->stick_conds = malloc(data->num * sizeof(pthread_cond_t *));
	if (!data->stick_conds)
	{
		fprintf(stderr, "Error: Failed to allocate memory for stick_conds.\n");
		exit(1);
	}


	data->print_lock = malloc(data->num * sizeof(pthread_mutex_t));
	if (!data->print_lock)
	{
		fprintf(stderr, "Error: Failed to allocate memory for print_lock.\n");
		exit(1);
	}

	data->blocktime = malloc(data->num * sizeof(double));
	if (!data->blocktime)
	{
		fprintf(stderr, "Error: Failed to allocate memory for blocktime.\n");
		exit(1);
	}

	data->start_hungry = malloc(data->num * sizeof(double));
	if (!data->start_hungry)
	{
		fprintf(stderr, "Error: Failed to allocate memory for start_hungry.\n");
		exit(1);
	}

	pthread_mutex_init(data->lock, NULL);
	pthread_mutex_init(data->print_lock, NULL);

	for (i = 0; i < data->num; i++)
	{
		ids[i] = i;
		data->stick_states[i] = -1;
		data->phil_states[i] = 'T';
		data->stick_conds[i] = malloc(data->num * sizeof(pthread_cond_t));
		if (!data->stick_conds[i])
		{
			fprintf(stderr, "Error: Failed to allocate memory for data->stick_conds[i].\n");
			exit(1);
		}

		pthread_cond_init(data->stick_conds[i], NULL);
		data->blocktime[i] = 0;
		data->start_hungry[i] = 0;
	}

	printf("#-Philosophers: %d\n", data->num);
	fflush(stdout);

	gettimeofday(&tv, NULL);
	data->dt0 = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

	srandom(seed);

	if (data->accounting_interval > 0)
	{
		if (pthread_create(&atid, NULL, accounting_thread, (void *) data) != 0)
		{
			fprintf(stderr, "Error: Failed to create accounting thread.\n");
			exit(1);
		}
		pthread_detach(atid);
	}

	data->v = initialize_philo(data->num);
	if (!data->v)
	{
		fprintf(stderr, "Error: Failed to initialize philosopher structure.\n");
		exit(1);
	}

	for (i = 0; i < data->num; i++)
	{
		void **args = malloc(3 * sizeof(void *)); // Allocate memory for argument array
		if (!args)
		{
			fprintf(stderr, "Error: Failed to allocate memory for philosopher args.\n");
			exit(1);
		}

		args[0] = data;  // Shared data structure
		args[1] = data->v;  // Philosopher control structure
		args[2] = &ids[i];  // Philosopher ID

		if (pthread_create(&tids[i], NULL, philosopher, args) != 0)
		{
			fprintf(stderr, "Error: Failed to create philosopher thread %d.\n", i);
			exit(1);
		}
		pthread_detach(tids[i]);
	}

	pthread_exit(NULL);

}
