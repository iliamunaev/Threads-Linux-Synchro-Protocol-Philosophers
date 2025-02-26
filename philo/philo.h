#ifndef PHILO_H
# define PHILO_H

#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


#define THINKING  (0x7fffffff) // Max integer value: represents thinking
#define EATING    (-1)         // Eating is represented by -1 (lowest priority)
#define THRESHOLD       (5)    // Defines starvation threshold
// #define THRESHOLD (3)


typedef struct s_data
{
  int num;
  int maxthink;
  int time_to_eat;
  int time_to_sleep;
  int sleep;    /* 'U' or 'S' */
  pthread_mutex_t *lock;

  int *phil_states;

  long t0;
  long dt0;

  pthread_mutex_t *print_lock;
  double *blocktime;
  double *start_hungry;
  int accounting_interval;

  void *v;
} t_data;

typedef struct s_philo
{
  int num;
  pthread_mutex_t *lock;
  pthread_cond_t **blocked_philosophers;
  pthread_mutex_t *forks_lock;

  int *philo_num;
  int counter;
} t_philo;



int main(int argc, char **argv);
void *philosopher(void *arg);
char *phil_time(t_data *data);
void *accounting_thread(void *arg);
void pick_up_fork(t_philo *philo, int id);
void put_down_fork(t_philo *philo, int id);

t_philo *initialize_philo(int phil_count);
void i_am_done_eating( t_philo *philo, int philosopher);
void i_am_hungry(t_philo *philo, int philosopher);

# endif
