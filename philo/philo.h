#ifndef PHILO_H
# define PHILO_H

#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


// struct phil
// {
//   int num;
//   int maxthink;
//   int maxeat;
//   int sleep;    /* 'U' or 'S' */
//   int print;    /* 'Y' or 'N' */

//   pthread_mutex_t *lock;
//   pthread_cond_t **stick_conds;
//   int *stick_states;
//   int *phil_states;

//   long t0;
//   double dt0;

//   pthread_mutex_t *blocklock;
//   double *blocktime;
//   double *start_hungry;
//   int accounting_interval;

//   void *v;
// };


typedef struct s_data
{
  int num;
  int maxthink;
  int maxeat;
  int sleep;    /* 'U' or 'S' */
  int print;    /* 'Y' or 'N' */

  pthread_mutex_t *lock;
  pthread_cond_t **stick_conds;
  int *stick_states;
  int *phil_states;

  long t0;
  long dt0;

  pthread_mutex_t *blocklock;
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
  int *phil_number;
  int counter;
} t_philo;

#define HIGH_SENTINEL (0x7fffffff)
#define LOW_SENTINEL (-1)
#define THRESH (5)

int main(int argc, char **argv);
void *philosopher(void *arg);
char *phil_time(t_data *data);
void *accounting_thread(void *arg);
void pick_up_chopstick(t_data *data, int id, int stick);
void put_down_chopstick(t_data *data, int id, int stick);

t_philo *initialize_philo(int phil_count);
void i_am_hungry(t_data *data, t_philo *philo, int philosopher);
void i_am_done_eating(t_data *data, t_philo *philo, int philosopher);

# endif
