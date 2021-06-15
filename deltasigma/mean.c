#include <stdio.h>
#include <stdlib.h>

#define LBUFSIZE 128
#define MAXN 300

void filter_mean(int N)
{
    int t;
    double x, y;
    char lbuf[LBUFSIZE];

    int i;
    double past_x[MAXN];
    double sum;

    for (i = 0; i < MAXN; i++) {
	past_x[i] = 0.0;
    }

    while (fgets(lbuf, sizeof(lbuf), stdin)) {
	/* 信号データ入力 */
	sscanf(lbuf, "%d %lf", &t, &x);
	/* 過去データを１つシフトする */
	for (i = 0; i < N-1; i++) {
	    past_x[i] = past_x[i+1];
	}
	past_x[N-1] = x;
	/* 平均を求める */
	sum = 0.0;
	for (i = 0; i < N; i++) {
	    sum += past_x[i];
	}
	y = sum / N;
	/* 結果を出力 */
	printf("%d %e\n", t, y);
    }
}

int main(int argc, char **argv)
{
    int N;

    if (argc != 2) {
	fprintf(stderr, "usage: mean <N>\n");
	exit(2);
    }
    N = atoi(argv[1]);
    if (N < 1 || N > MAXN) {
	fprintf(stderr, "error: too large or smaller parameter of N\n");
	exit(2);
    }

    filter_mean(N);

    return (0);
}
