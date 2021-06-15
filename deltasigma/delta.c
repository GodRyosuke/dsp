#include <stdio.h>
#include <stdlib.h>

#define LBUFSIZE 128

#define COEFF 0.125

void do_delta_dac(void)
{
    int t;
    double x, y;
    char lbuf[LBUFSIZE];

    double integ;
    double u;

    integ = 0.0; u = 0.0;

    while (fgets(lbuf, sizeof(lbuf), stdin)) {
	sscanf(lbuf, "%d %lf", &t, &x);
	integ += u * COEFF;
	if (x - integ >= 0) u =  1.0;
	else                u = -1.0;
	y = integ;
	printf("%d %lf\n", t, y);
    }
}

int main(int argc, char **argv)
{
    if (argc != 1) {
	fprintf(stderr, "usage: delta\n");
	fprintf(stderr, "ex. wave_sin 500 0 1000 | delta | mean 100 | xgraph\n");
	exit(2);
    }

    do_delta_dac();

    return (0);
}
