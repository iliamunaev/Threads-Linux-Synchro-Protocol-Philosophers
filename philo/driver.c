#include "philo.h"

// Function to handle when a philosopher becomes hungry and wants to eat
void i_am_hungry(t_philo *philo, int id) {
    while (1) {
        if (id % 2 == 0) {  // Even philosophers pick left first
            if (pthread_mutex_trylock(philo->left_fork_lock[id]) == 0) {  // Try left fork
                if (pthread_mutex_trylock(philo->right_fork_lock[id]) == 0) {  // Try right fork
                    break;  // Success, exit loop
                } else {
                    pthread_mutex_unlock(philo->left_fork_lock[id]); // Release left if right isn't free
                }
            }
        } else {  // Odd philosophers pick right first
            if (pthread_mutex_trylock(philo->right_fork_lock[id]) == 0) {  // Try right fork
                if (pthread_mutex_trylock(philo->left_fork_lock[id]) == 0) {  // Try left fork
                    break;  // Success, exit loop
                } else {
                    pthread_mutex_unlock(philo->right_fork_lock[id]); // Release right if left isn't free
                }
            }
        }
        usleep(100); // Prevent CPU overload while waiting
    }
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
	put_down_fork(philo, philosopher, philosopher);
	put_down_fork(philo, philosopher, (philosopher + 1) % philo->num);

	// Reset the philosopher's queue number to a high sentinel value (indicating they are not hungry)
	philo->philo_num[philosopher] = THINKING;

	// Signal the left and right neighbors that forks might be available
	pthread_cond_signal(philo->blocked_philosophers[left_philo]);
	pthread_cond_signal(philo->blocked_philosophers[right_philo]);

	pthread_mutex_unlock(philo->lock); // Unlock the mutex after updating philosopher states
}

#include "philo.h"


// Function to pick up a fork
void pick_up_fork(t_philo *philo, int id, int fork)
{
	pthread_mutex_t *fork_lock;

	fork_lock = (fork == id) ? philo->left_fork_lock[id] : philo->right_fork_lock[id];

	if (id % 2 == 0) {
		pthread_mutex_lock(philo->left_fork_lock[id]);  // Pick left first
		pthread_mutex_lock(philo->right_fork_lock[id]); // Pick right second
	} else {
		pthread_mutex_lock(philo->right_fork_lock[id]); // Pick right first
		pthread_mutex_lock(philo->left_fork_lock[id]);  // Pick left second
	}



	// pthread_mutex_lock(data->print_lock);
	// printf("Philosopher %d: Trying to pick up fork %d\n", id, fork);
	// pthread_mutex_unlock(data->print_lock);

	pthread_mutex_lock(fork_lock); // Lock the mutex to safely modify shared resources

	// printf("Philosopher %d: Successfully picked up fork %d\n", id, fork);

	fflush(stdout);

}

// Function to put down a fork
void put_down_fork(t_philo *philo, int id, int fork)
{
	pthread_mutex_t *fork_lock;
	fork_lock = (fork == id) ? philo->left_fork_lock[id] : philo->right_fork_lock[id];

	// printf("Philosopher %d: Putting down fork %d\n", id, fork);

	pthread_mutex_unlock(fork_lock); // Release the fork


	// printf("Philosopher %d Released Fork %d\n", id, fork);
	fflush(stdout);
}
