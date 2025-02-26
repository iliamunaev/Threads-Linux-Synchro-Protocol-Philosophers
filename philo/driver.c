#include "philo.h"


void i_am_hungry(t_philo *philo, int philosopher)
{
    int left_philo = (philosopher + philo->num - 1) % philo->num;
    int right_philo = (philosopher + 1) % philo->num;

    pthread_mutex_lock(philo->lock);
    // Assigning a hunger priority
    philo->philo_num[philosopher] = philo->counter++;

    // Wait if neighbors are starving
    while (philo->philo_num[philosopher] - philo->philo_num[left_philo] > THRESHOLD * philo->num ||
		philo->philo_num[philosopher] - philo->philo_num[right_philo] > THRESHOLD * philo->num)
	{
        pthread_cond_wait(philo->blocked_philosophers[philosopher], philo->lock);
    }

    pick_up_fork(philo, philosopher);
    // // pick_up_fork(philo, (philosopher + 1) % philo->num);


    pthread_mutex_unlock(philo->lock);
}

// Function to handle when a philosopher finishes eating
void i_am_done_eating( t_philo *philo, int philosopher)
{
	int left_philo, right_philo;

	 // Determine the left and right philosophers based on circular arrangement
	left_philo = (philosopher + philo->num - 1) % philo->num;
	right_philo = (philosopher + 1) % philo->num;

	pthread_mutex_lock(philo->lock); // Lock the mutex before modifying shared resources

	// printf("Philosopher %d: Finished eating, releasing forks\n", philosopher);

	// Put down both forks after eating
	put_down_fork(philo, philosopher);
	// put_down_fork(philo, (philosopher + 1) % philo->num);

	// Reset the philosopher's queue number to a high sentinel value (indicating they are not hungry)
	philo->philo_num[philosopher] = THINKING;

	// Signal the left and right neighbors that forks might be available
	pthread_cond_signal(philo->blocked_philosophers[left_philo]);
	pthread_cond_signal(philo->blocked_philosophers[right_philo]);

	pthread_mutex_unlock(philo->lock); // Unlock the mutex after updating philosopher states
}

// Function to pick up a fork
// void pick_up_fork(t_philo *philo, int id)
// {

//     int left_fork = id;
//     int right_fork = (id + 1) % philo->num;  // Next fork in circular array


// 	// printf("[DEBUG] Philosopher %d is trying to pick up fork %d\n", id, left_fork);

// 	if (pthread_mutex_lock(&philo->forks_lock[left_fork]) != 0)
// 		return ;

// 	// printf("[DEBUG] Philosopher %d is trying to pick up fork %d\n", id, right_fork);

// 	if (pthread_mutex_lock(&philo->forks_lock[right_fork]) != 0)
// 	{
// 		pthread_mutex_unlock(&philo->forks_lock[left_fork]);
// 		return ;
// 	}

// 	// printf("[DEBUG] Philosopher %d is picked up forks successfully\n", id);

// }
void pick_up_fork(t_philo *philo, int philosopher)
{
    int first_fork, second_fork;


    if (philosopher % 2 == 0) {
        first_fork = philosopher;                  // Even philosophers pick left first
        second_fork = (philosopher + 1) % philo->num;
    } else {
        first_fork = (philosopher + 1) % philo->num; // Odd philosophers pick right first
        second_fork = philosopher;
    }


    pthread_mutex_lock(&philo->forks_lock[first_fork]);

    pthread_mutex_lock(&philo->forks_lock[second_fork]);
}

void put_down_fork(t_philo *philo, int id)
{
    int left_fork = id;
    int right_fork = (id + 1) % philo->num;

    pthread_mutex_unlock(&philo->forks_lock[left_fork]);
    pthread_mutex_unlock(&philo->forks_lock[right_fork]);
}


// void put_down_fork(t_philo *philo, int id)
// {

//     int left_fork = id;
//     int right_fork = (id + 1) % philo->num;  // Next fork in circular array

// 	// printf("[DEBUG] Philosopher %d is trying to put down fork %d\n", id, left_fork);

// 	pthread_mutex_unlock(&philo->forks_lock[left_fork]);

// 	// printf("[DEBUG] Philosopher %d is trying to put down fork %d\n", id, right_fork);

// 	pthread_mutex_unlock(&philo->forks_lock[right_fork]);

// 	// printf("[DEBUG] Philosopher %d is put down forks successfully\n", id);

// }

