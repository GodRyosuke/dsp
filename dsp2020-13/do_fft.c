/* --------------------------------------------------- */
/* DFT/FFTプログラム                                  */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

// コンパイル方法
// gcc -O -Wall do_fft.c -o do_fft -lm
// ln –s do_fft do_ifft
// ln –s do_fft do_win

// シンボリックリンクを張ってFFT/IFFT/WINの動作を１個のプログラムで共用する
// サンプル数が2^kではないとき自動的にDFTになる
// サンプル区切りは空白行またはファイル毎
// マイナス周波数成分がグラフ表示の左側に表示されるように出力順を変えている
// IFFT時に注意すること

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <math.h>

// #define OFFSET 0
#define OFFSET 1e-10

#define LBUFSIZE 128

enum { FFT, IFFT, WIN };

// エラー処理付きmalloc()関数
void *m_alloc(size_t size)
{
  void *p;

  p = malloc(size);
  if (!p) {
    fprintf(stderr, "error: not enough memory\n");
    exit(2);
  }

  return (p);
}

// エラー処理付きrealloc()関数
void *re_alloc(void *p, size_t size)
{
  p = realloc(p, size);
  if (!p) {
    fprintf(stderr, "error: not enough memory\n");
    exit(2);
  }

  return (p);
}

// FFTまたはIFFT処理 (データ個数nが2^k個のとき)
void fft(double *x, double *y, double *a, double *b, int n, double f)
{
  int i, j, k;
  double temp;
  int i0, i1, l;
  int n1, ns; 
  double c1, s1, c, s, sc;
  double a1, b1;

  // 変換後のバッファを作業領域とする
  memcpy(a, x, n * sizeof(*a));
  memcpy(b, y, n * sizeof(*b));
  
  j = 0;
  for (i = 0; i < n - 1; i++) {
    if (i <= j) {
      temp = a[i]; a[i] = a[j]; a[j] = temp;
      temp = b[i]; b[i] = b[j]; b[j] = temp;
    }
    k = n / 2;
    while (k <= j) {
      j -= k;
      k /= 2;
    }
    j += k;
  }
  
  n1 = n / 2;
  ns = 1;
  sc = M_PI;
  while (ns <= n1) {
    c1 = cos(sc);
    s1 = sin(sc * f);
    c  = 1.0;
    s  = 0.0;
    for (l = 0; l < ns; l++) {
      for (i0 = l; i0 < n; i0 += ns * 2) {
        i1 = i0 + ns;
        a1 = a[i1] * c - b[i1] * s;
        b1 = a[i1] * s + b[i1] * c;
        a[i1] = a[i0] - a1;
        b[i1] = b[i0] - b1;
        a[i0] += a1;
        b[i0] += b1;
      }
      temp = c1 * c - s1 * s;
      s    = s1 * c + c1 * s;
      c    = temp;
    }
    ns *= 2;
    sc /= 2.0;
  }
  
  if (f < 0.0) {
    for (i = 0; i < n; i++) {
      a[i] /= (double)n;
      b[i] /= (double)n;
    }
  }
}

// DFTまたはIDFT処理 (データ個数nが2^k個ではないとき)
void dft(double *x, double *y, double *a, double *b, int n, double f)
{
  int i, j, k;
  double *c, *s;
  double p;
  
  c = (double *)m_alloc(sizeof(double) * n);
  s = (double *)m_alloc(sizeof(double) * n);
  
  p = M_PI * 2.0 / n;

  // cos, sinの配列表をあらかじめ作成しておく (計算の高速化のため)
  for (k = 0; k < n; k++) {
    c[k] = cos(p * k);
    s[k] = sin(p * k * f);
  }

  // DFT/IDFT処理  
  for (k = 0; k < n; k++) {
    a[k] = b[k] = 0;
    i = 0;
    for (j = 0; j < n; j++) {
      // ∑計算
      a[k] += x[j] * c[i] - y[j] * s[i];
      b[k] += x[j] * s[i] + y[j] * c[i];
      // i = (i + k) % n の計算（iの値はi = (k * j) % nとなる ）
      i += k;
      if (i >= n) {
        i -= n;
      }
    }
  }
  
  if (f < 0.0) {
    for (k = 0; k < n; k++) {
      a[k] /= (double)n;
      b[k] /= (double)n;
    }
  }
  
  free(c);
  free(s);
}

// 窓関数処理
void do_win(double *x, double *y, double *a, double *b, int n)
{
  int i;
  double rad, w;
  
  for (i = 0; i < n; i++) {
    rad = 2.0 * M_PI / (double)n * (double)i;
    switch (1) {
    case 0: w = 1.0; break; // 矩形窓
    case 1: w = 0.5  - 0.5  * cos(rad); break; // ハニング窓 (ハン窓)
    case 2: w = 0.54 - 0.46 * cos(rad); break; // ハミング窓
    case 3: w = 0.42 - 0.5  * cos(rad) + 0.08 * cos(2.0 * rad); break; // ブラックマン窓
    }
    a[i] = x[i] * w;
    b[i] = y[i] * w;
  }
}

// nが2^kかどうかを判定する
int ispow2(int n)
{
  if (n == 0) {
    return (0);
  }
  
  while ((n & 1) == 0) {
    n = (n >> 1);
  }
  
  return (n == 1);
}

// FFT/IFFT/DFT/IDFT処理
void do_fft(double *x, double *y, double *a, double *b, int n, double f)
{
  if (ispow2(n)) fft(x, y, a, b, n, f); // n==2^kのときはFFT/IFFT処理を実行する
  else           dft(x, y, a, b, n, f); // n!=2^kのときはDFT/IDFT処理を実行する
}

void _do_fft_fp(FILE *fp, int type)
{
  int i;
  char lbuf[LBUFSIZE], buf[LBUFSIZE], *p1, *p2, *p3;
  double v, *x, *y, *a, *b;
  int count, inbuf;
  static char *tok = " \t\n";
  
  x = y = NULL;
  count = 0;
  inbuf = 0;
  while (fgets(lbuf, sizeof(lbuf), fp)) {
    memcpy(buf, lbuf, sizeof(lbuf));
    p1 = strtok(buf, tok);
    p2 = strtok((char *)0, tok);
    p3 = strtok((char *)0, tok);
    if (!p1 || *p1 == ';' || *p1 == '#' || *p1 == '\"') { // "
      if (!count) {
        // 冒頭の「空白行」「コメント行」「データセット名の行」はそのまま出力する
        fputs(lbuf, stdout);
        continue;
      } else {
        // 冒頭ではないときはデータセットの終了と見なす
        inbuf++;
        break;
      }
    }
    // データを格納するバッファを拡張する (初回は確保, 2回目以降は拡張)
    x = (double *)re_alloc(x, (count + 1) * sizeof(*x));
    y = (double *)re_alloc(y, (count + 1) * sizeof(*y));
    if (p2) {
      x[count] = atof(p2);
    } else {
      fprintf(stderr, "error: illegal date. assumed as zero\n");
      // exit(2);
      x[count] = 0.0;
    }
    if (p3) {
      y[count] = atof(p3); // 虚部(3列目)がある場合はyに代入していく
    } else {
      y[count] = 0.0;
    }
    count++;
  }
  
  // データが無いときは戻る
  if (!count) {
    free(x); // 注意: この2行を忘れるとメモリリークになる
    free(y);
    return;
  }

  // 結果を格納するバッファを確保する
  a = (double *)m_alloc(count * sizeof(*a));
  b = (double *)m_alloc(count * sizeof(*b));
  
  // バッファ処理を実行
  switch (type) {
  case FFT:  do_fft(x, y, a, b, count, -1.0); break;
  case IFFT: do_fft(x, y, a, b, count,  1.0); break;
  case WIN:  do_win(x, y, a, b, count); break;
  }

  // 処理結果を表示する
  if (/* type == FFT || */ type == IFFT || type == WIN) {
    // 通常順で出力する(窓関数乗算時)
    for (i = 0; i < count; i++) {
      v = sqrt(a[i] * a[i] + b[i] * b[i]) /* + OFFSET */;
      printf("%d %e %e %e\n", i, a[i], b[i], v);
    }
  } else {
    // マイナス周波数成分を先に出力する（IFFTのとき注意!!）
    for (i = 0; i < count; i++) {
      int j;
      if (i < count / 2) {
        j = i + count / 2;
      } else {
        j = i - count / 2;
      }
      v = sqrt(a[j] * a[j] + b[j] * b[j]) + OFFSET;
      printf("%d %e %e %e\n", i - count / 2, a[j], b[j], v);
    }
  }
  
  if (inbuf) {
    fputs(lbuf, stdout);
  }
  
  free(x);
  free(y);
  free(a);
  free(b);
}

void do_fft_fp(FILE *fp, int type)
{
  // ファイルの末尾まで処理を繰り返す
  while (!feof(fp)) {
    _do_fft_fp(fp, type);
  }
}

void do_fft_file(char *fname, int type)
{
  FILE *fp;
  
  fp = fopen(fname, "r");
  if (!fp) {
    fprintf(stderr, "%s not found\n", fname);
    return;
  }
  
  do_fft_fp(fp, type);
  
  fclose(fp);
}

int main(int argc, char **argv)
{
  int type;
  char *pname;
  
  // 実行コマンド名を取得する
  pname = strrchr(argv[0], '/');
  if (!pname) pname = argv[0];
  else pname++;

  // 実行コマンド名によって動作を切り替える
  type = FFT;
  if (strcmp(pname, "do_ifft") == 0) type = IFFT; // do_ifftで実行されたときは逆FFTする
  if (strcmp(pname, "do_win") == 0)  type = WIN; // do_winで実行されたときは窓関数をかける

  if (argc > 1) {
    while (--argc) {
      // 各ファイルに対して処理を行い標準出力に結果を出力する
      do_fft_file(*++argv, type);
    }
  } else {
    // 標準入力に対して処理を行い標準出力に結果を出力する
    do_fft_fp(stdin, type);
  }
  
  return (0);
}
