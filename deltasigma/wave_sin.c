#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void wave_sin(int T, int t1, int t2)
{
    int t;
    double x;

    for (t = t1; t <= t2; t++) {
	x = sin(2.0 * M_PI * t / T);
	printf("%d %f\n", t, x);
    }
}

int main(int argc, char **argv)
{
    int T, t1, t2;

    if (argc != 4) {
	fprintf(stderr, "usage: wave_sin <T> <t1> <t2>\n");
	exit(2);
    }
    T = atoi(argv[1]);
    t1 = atoi(argv[2]);
    t2 = atoi(argv[3]);

    wave_sin(T, t1, t2);

    return (0);
}
