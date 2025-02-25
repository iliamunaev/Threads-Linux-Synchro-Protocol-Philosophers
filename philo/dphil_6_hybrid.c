

#include "philo.h"

// Function to initialize the philosopher structure and its synchronization mechanisms
t_philo *initialize_philo(int phil_count)
{
	t_philo *philo;
	int i;
	// Allocate memory for the philosopher structure
	philo = (t_philo *) malloc(sizeof(t_philo));
	if (!philo)
	{
		fprintf(stderr, "Error: Failed to allocate memory for philo.\n");
		return NULL;
	}
	philo->num = phil_count; // Number of philosophers
	philo->counter = 0; // Counter for philosopher queueing order

	// Allocate and initialize mutex lock for thread synchronization
	philo->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(philo->lock, NULL);

	// Allocate memory for philosopher condition variables and their queue numbers
	philo->blocked_philosophers = (pthread_cond_t **) malloc(sizeof(pthread_cond_t *)*philo->num);
	philo->phil_number = (int *) malloc(sizeof(int)*philo->num);

	// Initialize each philosopher's condition variable and queue number
	for (i = 0; i < philo->num; i++)
	{
		philo->blocked_philosophers[i] = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
		if (!philo->blocked_philosophers[i])
		{
			fprintf(stderr, "Error: Failed to allocate memory for condition variable.\n");
			return NULL;
		}
		pthread_cond_init(philo->blocked_philosophers[i], NULL);
		philo->phil_number[i] = HIGH_SENTINEL; // Initially, all philosophers have a high sentinel value
	}
	return philo;
}

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
	philo->phil_number[philosopher] = philo->counter;
	philo->counter++;

	 // Wait if the philosopher's turn is too far ahead of the neighboring philosophers
	while (philo->phil_number[philosopher] - philo->phil_number[left_philo] > THRESH * philo->num
		|| philo->phil_number[philosopher] - philo->phil_number[right_philo] > THRESH * philo->num)
	{
		pthread_cond_wait(philo->blocked_philosophers[philosopher], philo->lock);
	}
	printf("Philosopher %d: Got permission to eat\n", philosopher);

	// Pick up both chopsticks before eating
	pick_up_chopstick(data, philosopher, philosopher);
	pick_up_chopstick(data,  philosopher, (philosopher + 1) % philo->num);

	// Mark the philosopher's state as eating by setting their queue number to a low sentinel value
	philo->phil_number[philosopher] = LOW_SENTINEL - THRESH * philo->num;

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
	philo->phil_number[philosopher] = HIGH_SENTINEL;

	// Signal the left and right neighbors that chopsticks might be available
	pthread_cond_signal(philo->blocked_philosophers[left_philo]);
	pthread_cond_signal(philo->blocked_philosophers[right_philo]);

	pthread_mutex_unlock(philo->lock); // Unlock the mutex after updating philosopher states
}
