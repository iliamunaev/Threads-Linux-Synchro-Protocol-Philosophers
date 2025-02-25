#include "philo.h"

// // Function to return the time in a formatted string
// char *phil_time(t_data *data)
// {
// 	static char buf[100]; // Ensure buffer is static to avoid returning a local pointer
// 	struct timeval tv;
// 	double t;

// 	// If the philosopher is sleeping, calculate the time since start
// 	if (data->sleep == 'S')
// 	{
// 		sprintf(buf, "%3ld", time(0)-data->t0);
// 	}
// 	else
// 	{
// 		gettimeofday(&tv, NULL); // Get current time
// 		t = tv.tv_usec;
// 		t /= 1000000.0;

// 		t += tv.tv_sec;  // Calculate time in seconds
// 		sprintf(buf, "%7.3lf", (t-data->dt0));
// 	}
// 	return buf;
// }

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


// char *phil_time(t_data *data)
// {
//     static char buf[100];
//     struct timeval tv;
//     long t;

//     gettimeofday(&tv, NULL);

//     // Convert time to milliseconds
//     t = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

//     // Format as a string and store in `buf`
//     sprintf(buf, "%7ld", t - data->dt0);

//     return buf;
// }
