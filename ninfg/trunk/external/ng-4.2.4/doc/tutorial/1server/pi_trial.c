/* This is sample program for Ninf-G        */
/* pi_trial() called from Ninf-G Executable */
#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
#define RANDOM_MAX RAND_MAX
#else
#define RANDOM_MAX 2147483647 /* 2**31 - 1 */
#endif

long pi_trial(int seed, long times)
{
    long l, counter = 0;
    char hostname[256];

    srandom(seed);

    for (l = 0; l < times; l++) {
        double x = (double)random() / RANDOM_MAX;
        double y = (double)random() / RANDOM_MAX;

        if (x * x + y * y < 1.0) {
            counter++;
        }
     }

    if (gethostname(hostname, 256) == -1) {
      perror("gethostname");
      exit(1);
    }

     printf("%s: counter = %ld, times = %ld\n", hostname, counter, times);
     return counter;
}
