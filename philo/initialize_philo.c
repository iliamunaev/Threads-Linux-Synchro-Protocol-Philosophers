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

	philo->philo_num = malloc(philo->num * sizeof(int));

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
		philo->philo_num[i] = THINKING; // Initially, all philosophers have a high sentinel value
	}
	return philo;
}
