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

	// Pick up both chopsticks before eating
	pick_up_chopstick(data, philosopher, philosopher);
	pick_up_chopstick(data,  philosopher, (philosopher + 1) % philo->num);

	// Mark the philosopher's state as eating by setting their queue number to a low sentinel value
	philo->philo_num[philosopher] = EATING - THRESHOLD * philo->num;

	pthread_mutex_unlock(philo->lock); // Unlock the mutex after acquiring chopsticks
}

// Function to handle when a philosopher finishes eating
void i_am_done_eating(t_data *data, t_philo *philo, int philosopher)
{
	int left_philo, right_philo;

	 // Determine the left and right philosophers based on circular arrangement
	left_philo = (philosopher + philo->num - 1) % philo->num;
	right_philo = (philosopher + 1) % philo->num;

	pthread_mutex_lock(philo->lock); // Lock the mutex before modifying shared resources

	printf("Philosopher %d: Finished eating, releasing sticks\n", philosopher);

	// Put down both chopsticks after eating
	put_down_chopstick(data, philosopher, philosopher);
	put_down_chopstick(data, philosopher, (philosopher + 1) % philo->num);

	// Reset the philosopher's queue number to a high sentinel value (indicating they are not hungry)
	philo->philo_num[philosopher] = THINKING;

	// Signal the left and right neighbors that chopsticks might be available
	pthread_cond_signal(philo->blocked_philosophers[left_philo]);
	pthread_cond_signal(philo->blocked_philosophers[right_philo]);

	pthread_mutex_unlock(philo->lock); // Unlock the mutex after updating philosopher states
}

#include "philo.h"


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

		pthread_mutex_lock(data->print_lock);
		printf("%s Philosopher %d Blocking on Stick %d\n", (char *)phil_time(data), id, stick);
		fflush(stdout);
		pthread_mutex_unlock(data->print_lock);

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

	pthread_mutex_lock(data->print_lock);
	printf("%s Philosopher %d Picked Up Stick %d\n", (char *)phil_time(data), id, stick);
	fflush(stdout);
	pthread_mutex_unlock(data->print_lock);


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
		(char *)phil_time(data), id, stick, data->stick_states[stick]);
		exit(1);
	}

	data->stick_states[stick] = -1;  // Mark chopstick as available

	// Print debug message
	pthread_mutex_lock(data->print_lock);
	printf("%s Philosopher %d Put Down Stick %d\n", phil_time(data), id, stick);
	fflush(stdout);
	pthread_mutex_unlock(data->print_lock);

	pthread_cond_signal(data->stick_conds[stick]); // Signal other threads waiting for the chopstick

	printf("Philosopher %d: Signaled other philosophers for stick %d\n", id, stick);
	pthread_mutex_unlock(data->lock); // Unlock the mutex
}
