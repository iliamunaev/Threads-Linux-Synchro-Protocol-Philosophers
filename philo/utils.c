#include "philo.h"

// Function to return the time in a formatted string
char *phil_time(t_data *data)
{
	static char buf[100]; // Ensure buffer is static to avoid returning a local pointer
	struct timeval tv;
	double t;

	// If the philosopher is sleeping, calculate the time since start
	if (data->sleep == 'S')
	{
		sprintf(buf, "%3ld", time(0)-data->t0);
	}
	else
	{
		gettimeofday(&tv, NULL); // Get current time
		t = tv.tv_usec;
		t /= 1000000.0;

		t += tv.tv_sec;  // Calculate time in seconds
		sprintf(buf, "%7.3lf", (t-data->dt0));
	}
	return buf;
}
