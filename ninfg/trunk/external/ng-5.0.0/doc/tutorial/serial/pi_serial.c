/* This is sample program for Ninf-G, calculate PI on just serial program */
#include <stdio.h>
#include <stdlib.h>

long pi_trial(int seed, long times);

int
main(int argc, char *argv[])
{
    long times, answer;

    if (argc != 2){
        fprintf(stderr, "USAGE: %s TIMES\n", argv[0]);
        exit(2);
    }
    times = atol(argv[1]);

    answer = pi_trial(0, times);

    /* Compute and display pi. */
    printf("PI = %f\n", 4.0 * ((double)answer / times));

    return 0;
}
