#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define USECS_PER_SEC 1000000

#define NSECS_PER_SEC 1000000000

class Stopwatch
{
private:
    FILE *stream;

    double wall;

    double rusage;

    double Wall()
    {
        double time;

// clock_gettime(2) needs to link to librt (-lrt). On POSIX systems
// which these functions are available, the symbol _POSIX_TIMERS is
// defined in <unistd.h> to a value greater than 0.
#if defined _POSIX_TIMERS && _POSIX_TIMERS > 0
# if not defined CLOCK_MONOTONIC_RAW // Since Linux 2.6.28.
#  if not defined CLOCK_MONOTONIC
#   error "Missing CLOCK_MONOTONIC. Upgrade your kernel or comment this out."
#  endif // Some systems only implement CLOCK_MONOTONIC.
#  define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
# endif
        struct timespec timespec;

        // If the NTP daemon updates the system clock during a timing run
        // gettimeofday will return the adjusted time.

        // CLOCK_MONOTONIC_RAW provides access to a raw hardware-based
        // time that is not subject to NTP adjustments.

        clock_gettime(CLOCK_MONOTONIC_RAW, &timespec);

        time =
            (double)timespec.tv_sec +
            (double)timespec.tv_nsec / NSECS_PER_SEC;
#else
        struct timeval timeval;

        gettimeofday(&timeval, NULL);

        time =
            (double)timeval.tv_sec +
            (double)timeval.tv_usec / USECS_PER_SEC;
#endif

        return time;
    }

    double Rusage()
    {
        struct rusage usage;
        double usertime;
        double systime;

        getrusage(RUSAGE_SELF, &usage);

        usertime =
            (double) usage.ru_utime.tv_sec +
            (double) usage.ru_utime.tv_usec / USECS_PER_SEC;

        systime =
            (double) usage.ru_stime.tv_sec +
            (double) usage.ru_stime.tv_usec / USECS_PER_SEC;

        return usertime + systime;
    }

    bool running;

public:
    Stopwatch(const char *context, FILE *_stream = stdout) :
        stream(_stream), wall(0.0), rusage(0.0), running(false) {
        start(context);
    }

    Stopwatch(FILE *_stream = stdout) :
        stream(_stream), wall(0.0), rusage(0.0), running(false) {}

    ~Stopwatch() {}

    void start(const char *context = NULL)
    {
        if (context && strlen(context) > 0)
            fprintf(stream, "%s\n", context);

        running = true;

        wall = Wall();
        rusage = Rusage();
    }

    void stop(const char *context = NULL)
    {
        if (running == false) {
            fprintf(stderr, "Error: Stopwatch is not running\n");
            return;
        }

        wall = Wall() - wall;
        rusage = Rusage() - rusage;

        if (context && strlen(context) > 0)
            fprintf(stream, "%s\n", context);

        fprintf(stream,
                "wall: %.8f seconds\n"
                "usr+sys: %.8f seconds\n",
                wall,
                rusage);

        running = false;
    }
};

#endif /* STOPWATCH_H */
