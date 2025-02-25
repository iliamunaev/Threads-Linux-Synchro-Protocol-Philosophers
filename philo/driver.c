#include "philo.h"

// Function to handle when a philosopher becomes hungry and wants to eat
void i_am_hungry(t_data *data, t_philo *philo, int philosopher)
{
	int left_philo, right_philo;

	// Determine the left and right philosophers based on circular arrangement
	left_philo = (philosopher + philo->num - 1) % philo->num;
	right_philo = (philosopher + 1) % philo->num;

	pthread_mutex_lock(philo->lock); // Lock the mutex to ensure thread-safe operations
	printf("Philosopher %d: Wants to eat, waiting for turn...\n", philosopher);

	 // Assign the current philosopher a unique counter value
	philo->philo_num[philosopher] = philo->counter;
	philo->counter++;

	 // Wait if the philosopher's turn is too far ahead of the neighboring philosophers
	while (philo->philo_num[philosopher] - philo->philo_num[left_philo] > THRESHOLD * philo->num
		|| philo->philo_num[philosopher] - philo->philo_num[right_philo] > THRESHOLD * philo->num)
	{
		pthread_cond_wait(philo->blocked_philosophers[philosopher], philo->lock);
	}
	printf("Philosopher %d: Got permission to eat\n", philosopher);
	pthread_mutex_unlock(philo->lock);

	// Pick up both forks before eating
	pick_up_fork(data, philosopher, philosopher);
	pick_up_fork(data,  philosopher, (philosopher + 1) % philo->num);

	// Mark the philosopher's state as eating by setting their queue number to a low sentinel value
	pthread_mutex_lock(philo->lock);
	philo->philo_num[philosopher] = EATING - THRESHOLD * philo->num;
	pthread_mutex_unlock(philo->lock); // Unlock the mutex after acquiring forks
}

// Function to handle when a philosopher finishes eating
void i_am_done_eating(t_data *data, t_philo *philo, int philosopher)
{
	int left_philo, right_philo;

	 // Determine the left and right philosophers based on circular arrangement
	left_philo = (philosopher + philo->num - 1) % philo->num;
	right_philo = (philosopher + 1) % philo->num;

	pthread_mutex_lock(philo->lock); // Lock the mutex before modifying shared resources

	printf("Philosopher %d: Finished eating, releasing forks\n", philosopher);

	// Put down both forks after eating
	put_down_fork(data, philosopher, philosopher);
	put_down_fork(data, philosopher, (philosopher + 1) % philo->num);

	// Reset the philosopher's queue number to a high sentinel value (indicating they are not hungry)
	philo->philo_num[philosopher] = THINKING;

	// Signal the left and right neighbors that forks might be available
	pthread_cond_signal(philo->blocked_philosophers[left_philo]);
	pthread_cond_signal(philo->blocked_philosophers[right_philo]);

	pthread_mutex_unlock(philo->lock); // Unlock the mutex after updating philosopher states
}

#include "philo.h"


// Function to pick up a fork
void pick_up_fork(t_data *data, int id, int fork)
{

	pthread_mutex_lock(data->lock); // Lock the mutex to safely modify shared resources

	// pthread_mutex_lock(data->print_lock);
	printf("Philosopher %d: Trying to pick up fork %d\n", id, fork);
	// pthread_mutex_unlock(data->print_lock);

	// Check if philosopher is hungry (must be in 'H' state to pick up fork)
	if (data->phil_states[id] != 'H')
	{
		printf("%s Error -- pick_up_fork(%d %d) called and philosopher's state is %c .\n",
		phil_time(data), id, fork, data->phil_states[id]);
		pthread_mutex_unlock(data->lock);
		exit(1);
	}

	// Wait until the fork is available (if not, block)

	while (data->fork_states[fork] != -1)
	{

		if (!data->fork_conds[fork])
		{
			fprintf(stderr, "Error: fork_conds[%d] is NULL.\n", fork);
			pthread_mutex_unlock(data->lock);
			exit(1);
		}
		pthread_cond_wait(data->fork_conds[fork], data->lock); // Wait for fork to be available
	}

	data->fork_states[fork] = id; // Assign fork to philosopher


	// pthread_mutex_lock(data->print_lock);
	printf("%s Philosopher %d Picked Up Fork %d\n", (char *)phil_time(data), id, fork);
	fflush(stdout);
	// pthread_mutex_unlock(data->print_lock);

	pthread_mutex_unlock(data->lock); // Unlock the mutex
}

// Function to put down a fork
void put_down_fork(t_data *data, int id, int fork)
{
	pthread_mutex_lock(data->lock); // Lock the mutex to safely modify shared resources

	printf("Philosopher %d: Putting down fork %d\n", id, fork);

	// Check if philosopher is eating (must be in 'E' state to put down fork)
	if (data->phil_states[id] != 'E')
	{
		printf("%s Error -- put_down_fork(%d %d) called and philosopher's state is %c .\n",
		phil_time(data), id, fork, data->phil_states[id]);
		pthread_mutex_unlock(data->lock);
		exit(1);
	}

	// Ensure the philosopher is holding the fork
	if (data->fork_states[fork] != id)
	{
		printf("%s Error -- put_down_fork(%d %d) called and fork state is %d .\n",
		(char *)phil_time(data), id, fork, data->fork_states[fork]);
		pthread_mutex_unlock(data->lock);
		exit(1);
	}

	data->fork_states[fork] = -1;  // Mark fork as available

	// Print debug message
	// pthread_mutex_lock(data->print_lock);
	printf("%s Philosopher %d Put Down Fork %d\n", phil_time(data), id, fork);
	fflush(stdout);
	// pthread_mutex_unlock(data->print_lock);

	pthread_cond_signal(data->fork_conds[fork]); // Signal other threads waiting for the fork

	// pthread_mutex_lock(data->print_lock);
	printf("Philosopher %d: Signaled other philosophers for fork %d\n", id, fork);
	// pthread_mutex_unlock(data->print_lock);

	pthread_mutex_unlock(data->lock); // Unlock the mutex
}
