#ifndef PHILO_H
# define PHILO_H

#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


struct phil {
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
  double dt0;

  pthread_mutex_t *blocklock;
  double *blocktime;
  double *start_hungry;
  int accounting_interval;

  void *v;
};

typedef struct {
  int num;
  pthread_mutex_t *lock;
  pthread_cond_t **blocked_philosophers;
  int *phil_number;
  int counter;
} MyPhil;

#define HIGH_SENTINEL (0x7fffffff)
#define LOW_SENTINEL (-1)
#define THRESH (5)



extern void pick_up_chopstick(int philosopher, int stick);
extern void put_down_chopstick(int philosopher, int stick);


extern void *initialize_v(int phil_count);
extern void i_am_hungry(void *v, int philosopher);
extern void i_am_done_eating(void *v, int philosopher);

# endif
