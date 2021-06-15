/* --------------------------------------------------- */
/* LPF,HPF,BRF,BPFのインパルス応答計算プログラム       */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

/* 2002.01.16 (Wed) */
/* 2003.01.14 revised */
/* 2009.01.12 revised */
/* 2014.01.13 (Tue) revised */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAXHSIZE 1024

// bash% gcc -Wall h_calc.c -o h_calc -lm; ./h_calc 3 1> h_calc.out 2> h_calc.inc; gcc -Wall convolution.c -o convolution; cat h_calc.out | h_ftoku | xgraph -ly 0,1.4 -m
// tcsh% gcc -Wall h_calc.c -o h_calc -lm; (./h_calc 3 >! h_calc.out) >&! h_calc.inc; gcc -Wall convolution.c -o convolution; cat h_calc.out | h_ftoku | xgraph -ly 0,1.4 -m


/* sinc関数 */
double sinc(double x)
{
  if (x == 0) {
    return (1.0);
  } else {
    return (sin(M_PI * x) / (M_PI * x));
  }
}

/* フィルタ特性を反転させる */
/* LPF特性の時,HPF(ハイパスフィルタ)のインパルス応答に変換することになる */
/* HPF_h[i] = delta[i] - LPF_h[i]; */
int reverse_h(double *h, int hsize)
{
  int i, offset;

  /* hsizeは対称性を保つために奇数にすること */
  offset = (hsize - 1) / 2;

  for (i = 0; i < hsize; i++) {
    h[i] = -h[i];
  }

  h[offset] += 1.0;  /* delta */

  return (hsize);
}

/* インパルス応答を0に初期化する */
int clear_h(double *h, int hsize)
{
  int i;

  for (i = 0; i < hsize; i++) {
    h[i] = 0;
  }

  return (hsize);
}

/* 波形合成 (add) */
int add_h(double *h, double *h1, double *h2, int hsize)
{
  int i;

  for (i = 0; i < hsize; i++) {
    h[i] = h1[i] + h2[i];
  }

  return (hsize);
}

/* 波形合成 (sub) */
int sub_h(double *h, double *h1, double *h2, int hsize)
{
  int i;

  for (i = 0; i < hsize; i++) {
    h[i] = h1[i] - h2[i];
  }

  return (hsize);
}

/* 波形合成 (multiple and add) h = h1 + k * h2 */
int muladd_h(double *h, double *h1, double *h2, double k, int hsize)
{
  int i;

  for (i = 0; i < hsize; i++) {
    h[i] = h1[i] + k * h2[i];
  }

  return (hsize);
}

/* LPF(ローパスフィルタ)のインパルス応答を計算する */
/* 帯域幅をBで与える */
int set_h_LPF(double *h, int hsize, double B)
{
  int i, offset;

  /* hsizeは対称性を保つために奇数にすること */
  offset = (hsize - 1) / 2;

  for (i = 0; i < hsize; i++) {
    h[i] = B * sinc(B * (i - offset));
  }

  return (hsize);
}

/* HPF(ハイパスフィルタ)のインパルス応答を計算する */
/* 帯域幅をBで与える */
int set_h_HPF(double *h, int hsize, double B)
{
  set_h_LPF(h, hsize, B);
  reverse_h(h, hsize);

  return (hsize);
}

/* BPF(バンドパスフィルタ)のインパルス応答を計算する */
/* 通過域をB1〜B2で与える */
int set_h_BPF(double *h, int hsize, double B1, double B2)
{
  double h1[MAXHSIZE], h2[MAXHSIZE];
  set_h_LPF(h1, hsize, B1);
  set_h_LPF(h2, hsize, B2);
  sub_h(h, h2, h1, hsize); /* h = h2 - h1 */

  return (hsize);
}

/* BRF(バンドリジェクションフィルタ)のインパルス応答を計算する */
/* 遮断域をB1〜B2で与える */
int set_h_BRF(double *h, int hsize, double B1, double B2)
{
  double h1[MAXHSIZE], h2[MAXHSIZE];
  set_h_LPF(h1, hsize, B1);
  set_h_HPF(h2, hsize, B2);
  add_h(h, h2, h1, hsize); /* h = h2 + h1 */

  return (hsize);
}

/* 各種フィルタのインパルス応答の導出 */
void calc_response(int type, double *h, int hsize)
{
  double h1[MAXHSIZE], h2[MAXHSIZE], h3[MAXHSIZE];

  switch (type) {
  case 1:
    /* --- 単純な低域通過フィルタ(LPF)の特性 --- */
    /* h  : B=0.05 の hsize-tap LPF のインパルス応答 */
    set_h_LPF(h, hsize, 0.1);
    break;

  case 2:
    /* --- 単純な高域通過フィルタ(HPF)の特性 --- */
    /* h  : B=0.5 の hsize-tap HPF のインパルス応答 */
    set_h_HPF(h, hsize, 0.5);
    // ↓と同意
    // set_h_HPF(h, hsize, 0.5);
    // reverse_h(h, hsize);
    break;

  case 3:
    /* --- 帯域遮断フィルタの特性(BRF) --- */
    /* h  : B=0.05〜0.5 の hsize-tap BRF のインパルス応答 */
    set_h_BRF(h, hsize, 0.05, 0.5);
    // ↓と同意
    // set_h_LPF(h1, hsize, 0.05);
    // set_h_LPF(h2, hsize, 0.5);
    // reverse_h(h2, hsize);
    // add_h(h, h1, h2, hsize);
    break;

  case 4:
    /* --- 帯域通過フィルタの特性(BPF) --- */
    /* h  : B=0.05〜0.5 の hsize-tap BPF のインパルス応答 */
    set_h_BPF(h, hsize, 0.05, 0.5);
    // ↓と同意
    // set_h_LPF(h1, hsize, 0.05);
    // set_h_LPF(h2, hsize, 0.5);
    // reverse_h(h2, hsize);
    // add_h(h, h1, h2, hsize);
    // reverse_h(h, hsize);
    break;

  case 5:
    /* ---  3バンド・イコライザ --- */
    set_h_LPF(h1, hsize, 0.05);
    set_h_BPF(h2, hsize, 0.05, 0.5);
    set_h_HPF(h3, hsize, 0.5);
    clear_h(h, hsize);
    muladd_h(h, h, h1, 1.0, hsize); /* 低域: 1.0倍 */
    muladd_h(h, h, h2, 0.4, hsize); /* 中域: 0.4倍 */
    muladd_h(h, h, h3, 0.8, hsize); /* 高域: 0.8倍 */
    break;

  case 0:
    /* --- 単純な低域通過フィルタ(LPF)の特性 --- */
    /* h  : B=0.5 の hsize-tap LPF のインパルス応答 */
    set_h_LPF(h, hsize, 0.5);
    break;

  default:
    fprintf(stderr, "error: unknown type number\n");
    exit(2);
  }
}

/* 窓関数 */
void mult_window(int win, double *h, int hsize)
{
    int i;
    double n, L, rad, w;

    for (i = 0; i < hsize; i++) {
      n = i - (hsize - 1) / 2;
      L = hsize + 1;
      rad = 2.0 * M_PI * n / (double)L;
      switch (win) {
      case 0: /* 方形窓 (Window処理なし) */
	w = 1.0;
	break;
      case 1: /* ハニング窓 */
	w = 0.5 + 0.5 * cos(rad);
	break;
      case 2: /* ハミング窓 */
	w = 0.54 + 0.46 * cos(rad);
	break;
      case 3: /* ブラックマン窓 */
	w = 0.42 + 0.5 * cos(rad) + 0.08 * cos(rad * 2.0);
	break;
      default:
	fprintf(stderr, "error: unknown window type\n");
	exit(2);
      }
      h[i] = h[i] * w;
    }
}

/* インパルス応答を表示する */
void show_response(FILE *out, double *h, int hsize)
{
  int i;
  char tmpbuf[256];

  /* for xgraph */
  for (i = 0; i < hsize; i++) {
    fprintf(out, "%d %f\n", i, h[i]);
  }
  fprintf(out, "\n");

  /* for c-program */
  fprintf(stderr, "    N = %d;\n", hsize);
  for (i = 0; i < hsize; i++) {
    // sprintf(tmpbuf, "h[%d]=%10.7f;", i, h[i]);
    sprintf(tmpbuf, "h[%d]=%10.4e;", i, h[i]);
    if (i % 4 == 0) fprintf(stderr, "    ");
    fprintf(stderr, "%-18s", tmpbuf);
    if (i % 4 == 3) fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
}

/* インパルス応答を求めて表示する */
void h_calc(FILE *out, int type, int hsize, int win)
{
  double h[MAXHSIZE];

  calc_response(type, h, hsize);
  mult_window(win, h, hsize);
  show_response(out, h, hsize);
}

/* main関数 */
int main(int argc, char **argv)
{
  int type;
  int hsize;
  int win;

  if (argc < 2 || argc > 4) {
    fprintf(stderr, "usage: h_calc <n> <hsize> <window.type>\n");
    exit(2);
  }

  type = atoi(argv[1]);

  if (argc >= 3) {
    hsize = atoi(argv[2]);
  } else {
    hsize = 31;
  }
  
  if (argc >= 4) {
    win = atoi(argv[3]);
  } else {
    win = 0;
  }
  
  if (hsize % 2 == 0) {
    fprintf(stderr, "// hsize must be odd\n");
    hsize++;
  }

  h_calc(stdout, type, hsize, win);

  return (0);
}
