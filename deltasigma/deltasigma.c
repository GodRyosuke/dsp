#include <stdio.h>
#include <stdlib.h>

#define LBUFSIZE 128

void deltasigma(void)
{
    int t;
    double x, y;
    char lbuf[LBUFSIZE];

    double integ;
    double u;

    integ = 0.0;
    u = 0.0;

    while (fgets(lbuf, sizeof(lbuf), stdin)) {
	sscanf(lbuf, "%d %lf", &t, &x);
	integ += (x - u);
	if (integ >= 0) u =  1.0;
	else            u = -1.0;
	y = u;
	printf("%d %f\n", t, y);
    }
}

int main(int argc, char **argv)
{
    if (argc != 1) {
	fprintf(stderr, "usage: dac\n");
	fprintf(stderr, "ex. wave_sin 500 0 1000 | dac | mean 100 | xgraph\n");
	exit(2);
    }

    deltasigma();

    return (0);
}
