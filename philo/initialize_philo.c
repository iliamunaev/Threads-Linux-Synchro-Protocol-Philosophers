#include "philo.h"

void init_forks(pthread_mutex_t *forks_lock, int num)
{
    for (int i = 0; i < num; i++) {
        if (pthread_mutex_init(&forks_lock[i], NULL) != 0)
		{
            fprintf(stderr, "Error: Failed to initialize fork mutex %d\n", i);
            exit(EXIT_FAILURE) ;
        }
    }
}
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
	philo->lock = malloc(sizeof(pthread_mutex_t));
	if (!philo->lock)
	{
		fprintf(stderr, "Error: lock malloc failed\n");
		exit(EXIT_FAILURE);
	}

	if (pthread_mutex_init(philo->lock, NULL) != 0)
	{
		fprintf(stderr, "Error: pthread_mutex_init failed\n");
		free(philo->lock);
		exit(EXIT_FAILURE);
	}



	philo->forks_lock = malloc(philo->num * sizeof(pthread_mutex_t));
	if (!philo->forks_lock)
	{
        fprintf(stderr, "Error: forks malloc failed\n");
        exit(EXIT_FAILURE);
    }
	init_forks(philo->forks_lock, philo->num);


	// Allocate memory for philosopher condition variables and their queue numbers
	philo->blocked_philosophers = (pthread_cond_t **) malloc(sizeof(pthread_cond_t *)*philo->num);
	if (!philo->blocked_philosophers)
	{
		fprintf(stderr, "Error: blocked_philosophers malloc failed\n");
		exit(EXIT_FAILURE);
	}

	philo->philo_num = malloc(philo->num * sizeof(int));
	if (!philo->philo_num)
	{
		fprintf(stderr, "Error: philo_num malloc failed\n");
		exit(EXIT_FAILURE);
	}

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
