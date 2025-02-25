
#include "philo.h"

// Function to return the time in a formatted string
char *phil_time(t_data *data)
{
	static char buf[100]; // Ensure buffer is static to avoid returning a local pointer
	struct timeval tv;
	double t;

	// If the philosopher is sleeping, calculate the time since start
	if (data->sleep == 'S')
	{
		sprintf(buf, "%3ld", time(0)-data->t0);
	}
	else
	{
		gettimeofday(&tv, NULL); // Get current time
		t = tv.tv_usec;
		t /= 1000000.0;

		t += tv.tv_sec;  // Calculate time in seconds
		sprintf(buf, "%7.3lf", (t-data->dt0));
	}
	return buf;
}
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
			usleep(data->accounting_interval); // Or use usleep if not sleeping
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

// Function to pick up a chopstick
void pick_up_chopstick(t_data *data, int id, int stick)
{

	pthread_mutex_lock(data->lock); // Lock the mutex to safely modify shared resources

	if (!data || !data->phil_states)
	{
		fprintf(stderr, "Error: Invalid data pointer in pick_up_chopstick.\n");
		exit(1);
	}

	printf("Philosopher %d: Trying to pick up stick %d\n", id, stick);
	// Check if philosopher is hungry (must be in 'H' state to pick up chopstick)
	if (data->phil_states[id] != 'H')
	{
		printf("%s Error -- pick_up_chopstick(%d %d) called and philosopher's state is %c .\n",
		phil_time(data), id, stick, data->phil_states[id]);
		exit(1);
	}

	// Wait until the chopstick is available (if not, block)
	while (data->stick_states[stick] != -1)
	{
		if (data->print == 'Y')
		{
			pthread_mutex_lock(data->blocklock);
			printf("%s Philosopher %d Blocking on Stick %d\n", phil_time(data), id, stick);
			fflush(stdout);
			pthread_mutex_unlock(data->blocklock);
		}
		if (!data->stick_conds[stick])
		{
			fprintf(stderr, "Error: stick_conds[%d] is NULL.\n", stick);
			exit(1);
		}
		pthread_cond_wait(data->stick_conds[stick], data->lock); // Wait for chopstick to be available
	}

	data->stick_states[stick] = id; // Assign chopstick to philosopher

	printf("Philosopher %d: Picked up stick %d\n", id, stick);
	// Print debug message if enabled
	if (data->print == 'Y')
	{
		pthread_mutex_lock(data->blocklock);
		printf("%s Philosopher %d Picked Up Stick %d\n", phil_time(data), id, stick);
		fflush(stdout);
		pthread_mutex_unlock(data->blocklock);
	}

	pthread_mutex_unlock(data->lock); // Unlock the mutex
}

// Function to put down a chopstick
void put_down_chopstick(t_data *data, int id, int stick)
{
	pthread_mutex_lock(data->lock); // Lock the mutex to safely modify shared resources

	printf("Philosopher %d: Putting down stick %d\n", id, stick);
	// Check if philosopher is eating (must be in 'E' state to put down chopstick)
	if (data->phil_states[id] != 'E')
	{
		printf("%s Error -- put_down_chopstick(%d %d) called and philosopher's state is %c .\n",
		phil_time(data), id, stick, data->phil_states[id]);
		exit(1);
	}

	// Ensure the philosopher is holding the chopstick
	if (data->stick_states[stick] != id)
	{
		printf("%s Error -- put_down_chopstick(%d %d) called and chopstick state is %d .\n",
		phil_time(data), id, stick, data->stick_states[stick]);
		exit(1);
	}

	data->stick_states[stick] = -1;  // Mark chopstick as available

	// Print debug message if enabled
	if (data->print == 'Y')
	{
		pthread_mutex_lock(data->blocklock);
		printf("%s Philosopher %d Put Down Stick %d\n", phil_time(data), id, stick);
		fflush(stdout);
		pthread_mutex_unlock(data->blocklock);
	}
	pthread_cond_signal(data->stick_conds[stick]); // Signal other threads waiting for the chopstick

	printf("Philosopher %d: Signaled other philosophers for stick %d\n", id, stick);
	pthread_mutex_unlock(data->lock); // Unlock the mutex
}

// Philosopher's behavior in a loop
void *philosopher(void *arg)
{

	t_data *data;
	t_philo *philo;
    int id;

	void **args = (void **) arg;
	data = (t_data *)args[0];  // Shared data structure
	philo = (t_philo *)args[1]; // Philosopher queue control structure
	id = *((int *)args[2]); // Philosopher ID

	free(args);  // Free the dynamically allocated memory for args

    printf("Philosopher %d started.\n", id); // Debug print


	int thinktime, eattime;
	struct timeval tv;
	double t;

	while (1)
	{
		// Philosopher thinks for a random amount of time
		thinktime = random() % data->maxthink + 1;

		pthread_mutex_lock(data->lock);
		if (data->print == 'Y')
		{
			pthread_mutex_lock(data->blocklock);
			printf("%s Philosopher %d Thinking (%d)\n", phil_time(data), id, thinktime);
			fflush(stdout);
			pthread_mutex_unlock(data->blocklock);
		}
		pthread_mutex_unlock(data->lock);

		pthread_mutex_lock(data->lock);
		if (data->sleep == 'U')
			usleep(thinktime);
		else
			sleep(thinktime);
		pthread_mutex_unlock(data->lock);


		// Philosopher becomes hungry
		pthread_mutex_lock(data->lock);
		if (data->print == 'Y')
		{
			pthread_mutex_lock(data->blocklock);
			printf("%s Philosopher %d Hungry\n", phil_time(data), id);
			fflush(stdout);
			pthread_mutex_unlock(data->blocklock);
		}
		pthread_mutex_unlock(data->lock);


		pthread_mutex_lock(data->lock);
		data->phil_states[id] = 'H'; // Set philosopher's state to hungry
		gettimeofday(&tv, NULL);
		t = tv.tv_usec;
		t /= 1000000.0;
		t += tv.tv_sec;
		data->start_hungry[id] = t; // Record the time when the philosopher becomes hungry
		pthread_mutex_unlock(data->lock);

		// pthread_mutex_lock(data->lock);
		i_am_hungry(data, philo, id); // Handle philosopher becoming hungry
		// pthread_mutex_unlock(data->lock);

		// Check if philosopher's state is still hungry (H) and chopstick state is correct
		pthread_mutex_lock(data->lock);
		if (data->phil_states[id] != 'H')
		{
			printf("%s Philosopher %d Error -- state should be H, but it's %c\n",
			phil_time(data), id, data->phil_states[id]);
			pthread_mutex_unlock(data->lock);
			exit(1);
		}
		pthread_mutex_unlock(data->lock);

		pthread_mutex_lock(data->lock);
		if (data->stick_states[id] != id)
		{
			printf("%s Philosopher %d Error -- stick %d state should be %d, but it is %d.\n",
			phil_time(data), id, id, id, data->stick_states[id]);
			pthread_mutex_unlock(data->lock);
			exit(1);
		}
		pthread_mutex_unlock(data->lock);

		// Get philosopher's food (stick) and start eating
		pthread_mutex_lock(data->lock);
		if (data->stick_states[(id+1)%data->num] != id)
		{
			printf("%s Philosopher %d Error -- stick %d state should be %d, but it is %d.\n",
			phil_time(data), id, (id+1)%data->num, id, data->stick_states[id]);
			pthread_mutex_unlock(data->lock);
			exit(1);
		}
		pthread_mutex_unlock(data->lock);


		pthread_mutex_lock(data->lock);
		data->phil_states[id] = 'E'; // Change state to eating
		gettimeofday(&tv, NULL);
		t = tv.tv_usec;
		t /= 1000000.0;
		t += tv.tv_sec;
		data->blocktime[id] += (t - data->start_hungry[id]); // Update the time spent eating
		pthread_mutex_unlock(data->lock);

		// Eating for a random amount of time
		eattime = random() % data->maxeat + 1;

		pthread_mutex_lock(data->lock);
		if (data->print == 'Y')
		{
			pthread_mutex_lock(data->blocklock);
			printf("%s Philosopher %d Eating (%d)\n", phil_time(data), id, eattime);
			fflush(stdout);
			pthread_mutex_unlock(data->blocklock);
		}
		pthread_mutex_unlock(data->lock);


		if (data->sleep == 'U')
			usleep(eattime);
		else
			sleep(eattime);

		// pthread_mutex_lock(data->lock);
		i_am_done_eating(data, philo, id); // Notify that the philosopher has finished eating
		// pthread_mutex_unlock(data->lock);

		pthread_mutex_lock(data->lock);
		if (data->phil_states[id] != 'E')
		{
			printf("%s Philosopher %d Error -- state should be E, but it's %c\n",
			phil_time(data), id, data->phil_states[id]);
			pthread_mutex_unlock(data->lock);
			exit(1);
		}
		pthread_mutex_unlock(data->lock);

		pthread_mutex_lock(data->lock);
		if (data->stick_states[id] == id)
		{
			printf("%s Philosopher %d Error -- stick %d state should not be %d, but it is.\n",
			phil_time(data), id, id, id);
			pthread_mutex_unlock(data->lock);
			exit(1);
		}
		pthread_mutex_unlock(data->lock);

		// Check the state of the other philosopher's chopstick

		pthread_mutex_lock(data->lock);
		if (data->stick_states[(id+1)%data->num] == id)
		{
			printf("%s Philosopher %d Error -- stick %d state should not be %d, but it is.\n",
			phil_time(data), id, (id+1)%data->num, id);
			pthread_mutex_unlock(data->lock);
			exit(1);
		}
		pthread_mutex_unlock(data->lock);

	}
}

// Function to display usage information
void usage(char *s)
{
	fprintf(stderr, "usage: dphil num-philosophers max-think max-eat accounting-interval seed(-1=time(0)) sleep(u|s) print(y|n)\n");
	if (strlen(s) > 0) fprintf(stderr, "%s\n", s);
	exit(1);
}

int main(int argc, char **argv)
{
	// Check if the number of arguments is correct
	if (argc != 8)
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
	data->maxeat = atoi(argv[3]); // Max eating time
	data->accounting_interval = atoi(argv[4]); // Accounting interval
	sscanf(argv[5], "%ld", &seed); // Get the random seed from the command line
	if (seed == -1)
		seed = time(0);  // Default seed if -1
	data->sleep = (argv[6][0] == 'u') ? 'U' : 'S'; // Set sleep mode (microseconds or seconds)
	data->print = (argv[7][0] == 'y') ? 'Y' : 'N'; // Enable or disable printing

	if (data->num <= 0 || data->maxthink <= 0 || data->maxeat <= 0)
	{
		usage("num-philosophers, max-think and max-eat all have to be greater than zero");
	}

	ids = malloc( data->num * sizeof(int));
	if (!ids)
	{
		fprintf(stderr, "Error: Failed to allocate memory for ids.\n");
		exit(1);
	}
	tids = (pthread_t *) malloc(sizeof(pthread_t) * data->num);
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


	data->blocklock = malloc(data->num * sizeof(pthread_mutex_t));
	if (!data->blocklock)
	{
		fprintf(stderr, "Error: Failed to allocate memory for blocklock.\n");
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
	pthread_mutex_init(data->blocklock, NULL);

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

	if (data->print == 'Y') printf("#-Philosophers: %d\n", data->num);
	fflush(stdout);

	data->t0 = time(0);
	gettimeofday(&tv, NULL);
	data->dt0 = tv.tv_usec;
	data->dt0 /= 1000000.0;
	data->dt0 += tv.tv_sec;

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
