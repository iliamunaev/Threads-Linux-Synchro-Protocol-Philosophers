#include "philo.h"

// Function to return the time in milliseconds as a formatted string
char *phil_time(t_data *data)
{
    static char buf[100];
    struct timeval tv;
    long t, relative_time;

    gettimeofday(&tv, NULL);

    // Convert absolute time to milliseconds
    t = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

    // Compute time relative to the start of the simulation
    relative_time = t - data->dt0;

    // Format the result as a string
    sprintf(buf, "%7ld", relative_time);

    return buf;
}
