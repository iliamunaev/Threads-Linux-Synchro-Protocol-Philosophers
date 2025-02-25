#include "philo.h"

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

    // printf("Philosopher %d started.\n", id); // Debug print

	int thinktime;
	struct timeval tv;
	double t;

	while (1)
	{
		// Philosopher thinks for a random amount of time
		thinktime = random() % data->maxthink + 1;

		pthread_mutex_lock(data->print_lock);
		printf("%s Philosopher %d Thinking (%d ms.)\n", (char *)phil_time(data), id, thinktime);
		fflush(stdout);
		pthread_mutex_unlock(data->print_lock);

		pthread_mutex_lock(data->lock);
		usleep(thinktime * 1000);
		pthread_mutex_unlock(data->lock);


		// Philosopher becomes hungry
		pthread_mutex_lock(data->print_lock);
		printf("%s Philosopher %d Hungry\n", (char *)phil_time(data), id);
		fflush(stdout);
		pthread_mutex_unlock(data->print_lock);

		pthread_mutex_lock(data->lock);
		data->phil_states[id] = 'H'; // Set philosopher's state to hungry
		gettimeofday(&tv, NULL);
		t = tv.tv_usec;
		t /= 1000000.0;
		t += tv.tv_sec;
		data->start_hungry[id] = t; // Record the time when the philosopher becomes hungry
		pthread_mutex_unlock(data->lock);

		// Handle philosopher becoming hungry
		i_am_hungry(data, philo, id);


		// Check if philosopher's state is still hungry (H) and chopstick state is correct
		pthread_mutex_lock(data->lock);
		if (data->phil_states[id] != 'H')
		{
			printf("%s Philosopher %d Error -- state should be H, but it's %c\n",
			(char *)phil_time(data), id, data->phil_states[id]);
			pthread_mutex_unlock(data->lock);
			exit(1);
		}
		pthread_mutex_unlock(data->lock);

		pthread_mutex_lock(data->lock);
		if (data->stick_states[id] != id)
		{
			printf("%s Philosopher %d Error -- stick %d state should be %d, but it is %d.\n",
			(char *)phil_time(data), id, id, id, data->stick_states[id]);
			pthread_mutex_unlock(data->lock);
			exit(1);
		}
		pthread_mutex_unlock(data->lock);

		// Get philosopher's food (stick) and start eating
		pthread_mutex_lock(data->lock);
		if (data->stick_states[(id+1)%data->num] != id)
		{
			printf("%s Philosopher %d Error -- stick %d state should be %d, but it is %d.\n",
			(char *)phil_time(data), id, (id+1)%data->num, id, data->stick_states[id]);
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

		pthread_mutex_lock(data->print_lock);
		printf("%s Philosopher %d start Eating (%d ms.)\n", (char *)phil_time(data), id, data->time_to_eat);
		fflush(stdout);
		pthread_mutex_unlock(data->print_lock);

		usleep(data->time_to_eat * 1000);

		// Notify that the philosopher has finished eating
		i_am_done_eating(data, philo, id);


		pthread_mutex_lock(data->lock);
		if (data->phil_states[id] != 'E')
		{
			printf("%s Philosopher %d Error -- state should be E, but it's %c\n",
			(char *)phil_time(data), id, data->phil_states[id]);
			pthread_mutex_unlock(data->lock);
			exit(1);
		}
		pthread_mutex_unlock(data->lock);

		pthread_mutex_lock(data->lock);
		if (data->stick_states[id] == id)
		{
			printf("%s Philosopher %d Error -- stick %d state should not be %d, but it is.\n",
			(char *)phil_time(data), id, id, id);
			pthread_mutex_unlock(data->lock);
			exit(1);
		}
		pthread_mutex_unlock(data->lock);

		// Check the state of the other philosopher's chopstick
		pthread_mutex_lock(data->lock);
		if (data->stick_states[(id+1)%data->num] == id)
		{
			printf("%s Philosopher %d Error -- stick %d state should not be %d, but it is.\n",
			(char *)phil_time(data), id, (id+1)%data->num, id);
			pthread_mutex_unlock(data->lock);
			exit(1);
		}
		pthread_mutex_unlock(data->lock);

	}
}

