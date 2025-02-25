
#include "philo.h"


struct phil P; // Declare a global philosopher structure
struct phil *PS; // Declare a pointer to the philosopher structure
char buf[100];

// Function to return the time in a formatted string
char *phil_time()
{
	struct timeval tv;
	double t;

	// If the philosopher is sleeping, calculate the time since start
	if (PS->sleep == 'S')
	{
		sprintf(buf, "%3ld", time(0)-PS->t0);
		return buf;
	}
	else
	{
		gettimeofday(&tv, NULL); // Get current time
		t = tv.tv_usec;
		t /= 1000000.0;
		t += tv.tv_sec;  // Calculate time in seconds
		sprintf(buf, "%7.3lf", (t-PS->dt0));
		return buf;
	}
}
// The accounting thread function to periodically track philosopher's states
void *accounting_thread(void *arg)
{
	(void)arg;
	double tblock;
	struct timeval tv;
	double t;
	int i;
	double *bt;// Array to store block time for each philosopher

	bt = (double *) malloc(sizeof(double)*PS->num);

	while (1)
	{
		if (PS->sleep == 'S')
		{
			sleep(PS->accounting_interval); // Sleep for the specified interval
		}
		else
		{
			usleep(PS->accounting_interval); // Or use usleep if not sleeping
		}
		gettimeofday(&tv, NULL);
		t = tv.tv_usec;
		t /= 1000000.0;
		t += tv.tv_sec; // Calculate total time

		tblock = 0;

		pthread_mutex_lock(PS->blocklock); // Lock the mutex for safe access to shared data
		for (i = 0; i < PS->num; i++)
		{
			bt[i] = PS->blocktime[i];
			if (PS->phil_states[i] == 'H')
				bt[i] += (t - PS->start_hungry[i]);
			tblock += bt[i];// Accumulate total block time
		}

		// Print the total and individual block times for philosophers
		printf("%s Total-Hungry %.3lf\n", phil_time(), tblock);
		printf("%s Individual-Hungry", phil_time());

		for (i = 0; i < PS->num; i++) printf(" %7.3lf", bt[i]);

		printf("\n");
		fflush(stdout); // Ensure output is written to console immediately
		pthread_mutex_unlock(PS->blocklock);
	}

	return NULL;
}

// Function to pick up a chopstick
void pick_up_chopstick(int id, int stick)
{
	// Check if philosopher is hungry (must be in 'H' state to pick up chopstick)
	if (PS->phil_states[id] != 'H')
	{
		printf("%s Error -- pick_up_chopstick(%d %d) called and philosopher's state is %c .\n",
		phil_time(), id, stick, PS->phil_states[id]);
		exit(1);
	}

	pthread_mutex_lock(PS->lock); // Lock the mutex to safely modify shared resources
	// Wait until the chopstick is available (if not, block)
	while (PS->stick_states[stick] != -1)
	{
		if (PS->print == 'Y')
		{
			pthread_mutex_lock(PS->blocklock);
			printf("%s Philosopher %d Blocking on Stick %d\n", phil_time(), id, stick);
			fflush(stdout);
			pthread_mutex_unlock(PS->blocklock);
		}
		pthread_cond_wait(PS->stick_conds[stick], PS->lock); // Wait for chopstick to be available
	}

	PS->stick_states[stick] = id; // Assign chopstick to philosopher

	// Print debug message if enabled
	if (PS->print == 'Y')
	{
		pthread_mutex_lock(PS->blocklock);
		printf("%s Philosopher %d Picked Up Stick %d\n", phil_time(), id, stick);
		fflush(stdout);
		pthread_mutex_unlock(PS->blocklock);
	}

	pthread_mutex_unlock(PS->lock); // Unlock the mutex
}

// Function to put down a chopstick
void put_down_chopstick(int id, int stick)
{
	// Check if philosopher is eating (must be in 'E' state to put down chopstick)
	if (PS->phil_states[id] != 'E')
	{
		printf("%s Error -- put_down_chopstick(%d %d) called and philosopher's state is %c .\n",
		phil_time(), id, stick, PS->phil_states[id]);
		exit(1);
	}

	// Ensure the philosopher is holding the chopstick
	if (PS->stick_states[stick] != id)
	{
		printf("%s Error -- put_down_chopstick(%d %d) called and chopstick state is %d .\n",
		phil_time(), id, stick, PS->stick_states[stick]);
		exit(1);
	}

	pthread_mutex_lock(PS->lock); // Lock the mutex
	PS->stick_states[stick] = -1;  // Mark chopstick as available

	// Print debug message if enabled
	if (PS->print == 'Y')
	{
		pthread_mutex_lock(PS->blocklock);
		printf("%s Philosopher %d Put Down Stick %d\n", phil_time(), id, stick);
		fflush(stdout);
		pthread_mutex_unlock(PS->blocklock);
	}
	pthread_cond_signal(PS->stick_conds[stick]); // Signal other threads waiting for the chopstick
	pthread_mutex_unlock(PS->lock); // Unlock the mutex
}

// Philosopher's behavior in a loop
void *philosopher(void *arg)
{
	int id;
	int thinktime, eattime;
	struct timeval tv;
	double t;

	id = *((int *) arg); // Get philosopher's id
	while (1)
	{
		// Philosopher thinks for a random amount of time
		thinktime = random() % PS->maxthink + 1;

		if (PS->print == 'Y')
		{
			pthread_mutex_lock(PS->blocklock);
			printf("%s Philosopher %d Thinking (%d)\n", phil_time(), id, thinktime);
			fflush(stdout);
			pthread_mutex_unlock(PS->blocklock);
		}

		if (PS->sleep == 'U')
			usleep(thinktime);
		else
			sleep(thinktime);

		// Philosopher becomes hungry
		if (PS->print == 'Y')
		{
			pthread_mutex_lock(PS->blocklock);
			printf("%s Philosopher %d Hungry\n", phil_time(), id);
			fflush(stdout);
			pthread_mutex_unlock(PS->blocklock);
		}

		pthread_mutex_lock(PS->blocklock);
		PS->phil_states[id] = 'H'; // Set philosopher's state to hungry
		gettimeofday(&tv, NULL);
		t = tv.tv_usec;
		t /= 1000000.0;
		t += tv.tv_sec;
		PS->start_hungry[id] = t; // Record the time when the philosopher becomes hungry
		pthread_mutex_unlock(PS->blocklock);

		i_am_hungry(PS->v, id); // Handle philosopher becoming hungry

		// Check if philosopher's state is still hungry (H) and chopstick state is correct
		if (PS->phil_states[id] != 'H')
		{
			printf("%s Philosopher %d Error -- state should be H, but it's %c\n",
			phil_time(), id, PS->phil_states[id]);
			exit(1);
		}
		if (PS->stick_states[id] != id)
		{
			printf("%s Philosopher %d Error -- stick %d state should be %d, but it is %d.\n",
			phil_time(), id, id, id, PS->stick_states[id]);
			exit(1);
		}

		// Get philosopher's food (stick) and start eating
		if (PS->stick_states[(id+1)%PS->num] != id)
		{
			printf("%s Philosopher %d Error -- stick %d state should be %d, but it is %d.\n",
			phil_time(), id, (id+1)%PS->num, id, PS->stick_states[id]);
			exit(1);
		}

		pthread_mutex_lock(PS->blocklock);
		PS->phil_states[id] = 'E'; // Change state to eating
		gettimeofday(&tv, NULL);
		t = tv.tv_usec;
		t /= 1000000.0;
		t += tv.tv_sec;
		PS->blocktime[id] += (t - PS->start_hungry[id]); // Update the time spent eating
		pthread_mutex_unlock(PS->blocklock);

		// Eating for a random amount of time
		eattime = random() % PS->maxeat + 1;

		if (PS->print == 'Y')
		{
			pthread_mutex_lock(PS->blocklock);
			printf("%s Philosopher %d Eating (%d)\n", phil_time(), id, eattime);
			fflush(stdout);
			pthread_mutex_unlock(PS->blocklock);
		}

		if (PS->sleep == 'U')
			usleep(eattime);
		else
			sleep(eattime);

		i_am_done_eating(PS->v, id); // Notify that the philosopher has finished eating

		if (PS->phil_states[id] != 'E')
		{
			printf("%s Philosopher %d Error -- state should be E, but it's %c\n",
			phil_time(), id, PS->phil_states[id]);
			exit(1);
		}
		if (PS->stick_states[id] == id)
		{
			printf("%s Philosopher %d Error -- stick %d state should not be %d, but it is.\n",
			phil_time(), id, id, id);
			exit(1);
		}
		// Check the state of the other philosopher's chopstick
		if (PS->stick_states[(id+1)%PS->num] == id)
		{
			printf("%s Philosopher %d Error -- stick %d state should not be %d, but it is.\n",
			phil_time(), id, (id+1)%PS->num, id);
			exit(1);
		}
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

	PS = &P; // Set pointer to the global philosopher structure
	PS->num = atoi(argv[1]); // Get the number of philosophers
	PS->maxthink = atoi(argv[2]); // Max thinking time
	PS->maxeat = atoi(argv[3]); // Max eating time
	PS->accounting_interval = atoi(argv[4]); // Accounting interval
	sscanf(argv[5], "%ld", &seed); // Get the random seed from the command line
	if (seed == -1)
		seed = time(0);  // Default seed if -1
	PS->sleep = (argv[6][0] == 'u') ? 'U' : 'S'; // Set sleep mode (microseconds or seconds)
	PS->print = (argv[7][0] == 'y') ? 'Y' : 'N'; // Enable or disable printing

	if (PS->num <= 0 || PS->maxthink <= 0 || PS->maxeat <= 0) {
		usage("num-philosophers, max-think and max-eat all have to be greater than zero");
	}

	ids = (int *) malloc(sizeof(int) * PS->num);
	tids = (pthread_t *) malloc(sizeof(pthread_t) * PS->num);
	PS->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * PS->num);
	PS->stick_states = (int *) malloc(sizeof(int) * PS->num);
	PS->phil_states = (int *) malloc(sizeof(int) * PS->num);
	PS->stick_conds = (pthread_cond_t **) malloc(sizeof(pthread_cond_t *) * PS->num);
	PS->blocklock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * PS->num);
	PS->blocktime = (double *) malloc(sizeof(double)*PS->num);
	PS->start_hungry = (double *) malloc(sizeof(double)*PS->num);

	pthread_mutex_init(PS->lock, NULL);
	pthread_mutex_init(PS->blocklock, NULL);

	for (i = 0; i < PS->num; i++) {
		ids[i] = i;
		PS->stick_states[i] = -1;
		PS->phil_states[i] = 'T';
		PS->stick_conds[i] = (pthread_cond_t *) malloc(sizeof(pthread_cond_t) * PS->num);
		pthread_cond_init(PS->stick_conds[i], NULL);
		PS->blocktime[i] = 0;
		PS->start_hungry[i] = 0;
	}

	if (PS->print == 'Y') printf("#-Philosophers: %d\n", PS->num);
	fflush(stdout);

	PS->t0 = time(0);
	gettimeofday(&tv, NULL);
	PS->dt0 = tv.tv_usec;
	PS->dt0 /= 1000000.0;
	PS->dt0 += tv.tv_sec;

	srandom(seed);

	if (PS->accounting_interval > 0) {
		pthread_create(&atid, NULL, accounting_thread, (void *) NULL);
		pthread_detach(atid);
	}

	PS->v = initialize_v(PS->num);
	for (i = 0; i < PS->num; i++) {
		pthread_create(tids+i, NULL, philosopher, (void *) (ids+i));
		pthread_detach(tids[i]);
	}
	pthread_exit(NULL);
}
