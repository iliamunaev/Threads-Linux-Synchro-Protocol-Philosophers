#include "philo.h"


// The accounting thread function to periodically track philosopher's states
void *accounting_thread(void *arg)
{
	t_data *data;
	data = (t_data *) arg; // Cast argument to `t_data *`
	if (!data)
	{
		fprintf(stderr, "Error: Null pointer received in accounting_thread.\n");
		return NULL;
	}

	double tblock;
	struct timeval tv;
	double t;
	int i;
	double *bt;// Array to store block time for each philosopher

	bt = (double *) malloc(sizeof(double)*data->num);
	if (!bt)
	{
		fprintf(stderr, "Memory allocation failed for bt array.\n");
		return NULL;
	}

	while (1)
	{
		if (data->sleep == 'S')
		{
			sleep(data->accounting_interval); // Sleep for the specified interval
		}
		else
		{
			usleep(data->accounting_interval + 1000); // Or use usleep if not sleeping
		}
		gettimeofday(&tv, NULL);
		t = tv.tv_usec;
		t /= 1000000.0;
		t += tv.tv_sec; // Calculate total time

		tblock = 0;

		if (!data->blocklock)
		{
			fprintf(stderr, "Error: blocklock is NULL in accounting_thread.\n");
			return NULL;
		}
		pthread_mutex_lock(data->blocklock); // Lock the mutex for safe access to shared data

		for (i = 0; i < data->num; i++)
		{
			bt[i] = data->blocktime[i];
			if (data->phil_states[i] == 'H')
				bt[i] += (t - data->start_hungry[i]);
			tblock += bt[i];// Accumulate total block time
		}

		// Print the total and individual block times for philosophers
		printf("%s Total-Hungry %.3lf\n", phil_time(data), tblock);
		printf("%s Individual-Hungry", phil_time(data));

		for (i = 0; i < data->num; i++) printf(" %7.3lf", bt[i]);

		printf("\n");
		fflush(stdout); // Ensure output is written to console immediately
		pthread_mutex_unlock(data->blocklock);
	}

	return NULL;
}
