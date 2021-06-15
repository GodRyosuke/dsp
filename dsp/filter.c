/* --------------------------------------------------- */
/* 各種ディジタルフィルタの処理プログラム              */
/*   LPF,HPF,BPF,BRF,EQ: 畳み込み積分計算              */
/*   ECHO              : 遅延に関する積和計算          */
/*                                                     */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

/* 2003.11.19 */
/* 2005.01.18 */
/* 2010.01.12 (Tue) */
/* 2014.01.13 (Tue) revised */

/* RIFFサブルーチンの読み込み */
#include "riff.c"

#define MAXHSIZE 8192
#define DELTA 1e-99

int opt_ftoku = 0; /* option: 周波数特性の表示     */
int opt_h     = 0; /* option: インパルス応答の表示 */
int opt_z     = 0; /* option: 畳み込み演算しない   */
int opt_c     = 0; /* option: カスケード接続数     */

/*
downconverter org-imai-44k.wav imai-44k.wav 1
downconverter org-kuraki-44k.wav kuraki-44k.wav 1
*/

/*
play imai-44k.wav
./downconvert imai-44k.wav imai-5k.wav 8
play imai-5k.wav 
./filter imai-44k.wav imai-LPF-44k.wav lpf 63 0.1 blackman
play imai-LPF-44k.wav 
./downconvert imai-LPF-44k.wav imai-LPF-5k.wav  8
play imai-LPF-5k.wav 
play imai-5k.wav 

play kuraki-44k.wav
./downconvert kuraki-44k.wav kuraki-5k.wav 8
play kuraki-5k.wav 
./filter kuraki-44k.wav kuraki-LPF-44k.wav lpf 63 0.1 blackman
play kuraki-LPF-44k.wav 
./downconvert kuraki-LPF-44k.wav kuraki-LPF-5k.wav  8
play kuraki-LPF-5k.wav 
play kuraki-5k.wav 
*/

/*
LPF,HPF,BPF,BRF,EQフィルタ処理
./filter -ftoku imai-44k.wav output.wav brf 255 0.03 0.3 blackman | xgraph -lny & sleep 9 ; play output.wav
./filter -ftoku imai-44k.wav output.wav bpf 255 0.03 0.3 blackman | xgraph -lny & sleep 9 ; play output.wav

./filter -ftoku kuraki-44k.wav output.wav brf 255 0.03 0.3 blackman | xgraph -lny & sleep 5 ; play output.wav
./filter -ftoku kuraki-44k.wav output.wav bpf 255 0.03 0.3 blackman | xgraph -lny & sleep 5 ; play output.wav

*/

/*
./filter imai-44k.wav echo.wav echo 3000 0.5 3500 0.2 5000 0.4 ; play echo.wav
./filter imai-44k.wav echo.wav echo 3000 0.4 3500 0.3 5000 0.2 ; play echo.wav
*/

/*
saxの倍音
filter -c2 sax-44k.wav sax-BPF.wav bpf 255 0.05 0.1 blackman ; play sax-BPF.wav
filter -c2 sax-44k.wav sax-BPF.wav bpf 255 0.1 0.2 blackman ; play sax-BPF.wav

ある音しか聴けない
filter -c1 sax-44k.wav sax-BPF.wav bpf 4095 0.04 0.045 blackman ; play sax-BPF.wav

帯域分離
filter -ftoku -c3 sax-44k.wav sax-LPF.wav lpf 255 0.05 blackman | xgraph -P -lny
filter -ftoku -c3 sax-44k.wav sax-HPF.wav hpf 255 0.05 blackman | xgraph -P -lny
( show sax-LPF.wav 25500 1000 ; echo "" ; show sax-HPF.wav 25500 1000 ) | xgraph =800x200 -M
show sax-44k.wav 25500 1000 | xgraph -M =800x200

 */

/*
高いピアノ音が聴えなくなる
filter -ftoku piano-44k.wav piano-LPF-44k.wav lpf 255 0.025 blackman > piano-LPF.xg
play piano-LPF-44k.wav
play piano-44k.wav
右手
filter piano-44k.wav piano-BPF-44k.wav bpf 1023 0.020 0.028 blackman
左手
filter piano-44k.wav piano-BPF-44k.wav bpf 1023 0.005 0.020 blackman
削除
filter -ftoku piano-44k.wav piano-BRF-44k.wav brf 1023 0.020 0.2 blackman | xgraph -lny
filter -ftoku -z piano-44k.wav piano-BRF-44k.wav brf 1023 0.020 0.2 blackman | xgraph -lny
play piano-BRF-44k.wav
 */

/* === FILTER-TYPE SUB-ROUTINES ======================================= */

int scan_string(char *key, char **list)
{
  int i;

  for (i = 0; list[i] != NULL; i++) {
    if (strcasecmp(key, list[i]) == 0) {
      return (i);
    }
  }
  return (-1);
}

enum { TYPE_MEAN, TYPE_LPF, TYPE_HPF, TYPE_BPF, TYPE_BRF, TYPE_DIFF, TYPE_ECHO, TYPE_EQ };
static char *typelist[] = { "mean", "lpf", "hpf", "bpf", "brf", "diff", "echo", "eq", NULL };

int scan_type(char *typename)
{
  int type;

  type = scan_string(typename, typelist);
  if (type < 0) {
    fprintf(stderr, "error: unknown type name '%s'\n", typename);
    exit(2);
  }
  return (type);
}

enum { WIN_RECT, WIN_HANNING, WIN_HAMMING, WIN_BLACKMAN };
static char *winlist[] = { "rect", "hanning", "hamming", "blackman", NULL };

int scan_win(char *winname)
{
  int win;

  win = scan_string(winname, winlist);
  if (win < 0) {
    fprintf(stderr, "error: unknown window name '%s'\n", winname);
    fprintf(stderr, "acceptable names: rect, hamming, hanning, blackman\n");
    exit(2);
  }
  return (win);
}

/* === IMPULSE RESPONSE =============================================-- */

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

/* フィルタ係数のグローバル変数 */
double BW, B1, B2;
double K1, K2, K3;

/* 各種フィルタのインパルス応答の導出 */
void calc_response(int type, double *h, int hsize)
{
  double h1[MAXHSIZE], h2[MAXHSIZE], h3[MAXHSIZE];

  switch (type) {
  case TYPE_LPF:
    /* --- 単純な低域通過フィルタ(LPF)の特性 --- */
    /* h  : B=BW の hsize-tap LPF のインパルス応答 */
    set_h_LPF(h, hsize, BW);
    break;

  case TYPE_HPF:
    /* --- 単純な高域通過フィルタ(HPF)の特性 --- */
    /* h  : B=BW の hsize-tap HPF のインパルス応答 */
    set_h_HPF(h, hsize, BW);
    // ↓と同意
    // set_h_HPF(h, hsize, BW);
    // reverse_h(h, hsize);
    break;

  case TYPE_BRF:
    /* --- 帯域遮断フィルタの特性(BRF) --- */
    /* h  : B=B1〜B2 の hsize-tap BRF のインパルス応答 */
    set_h_BRF(h, hsize, B1, B2);
    // ↓と同意
    // set_h_LPF(h1, hsize, B1);
    // set_h_LPF(h2, hsize, B2);
    // reverse_h(h2, hsize);
    // add_h(h, h1, h2, hsize);
    break;

  case TYPE_BPF:
    /* --- 帯域通過フィルタの特性(BPF) --- */
    /* h  : B=B1〜B2 の hsize-tap BPF のインパルス応答 */
    set_h_BPF(h, hsize, B1, B2);
    // ↓と同意
    // set_h_LPF(h1, hsize, B1);
    // set_h_LPF(h2, hsize, B2);
    // reverse_h(h2, hsize);
    // add_h(h, h1, h2, hsize);
    // reverse_h(h, hsize);
    break;

  case TYPE_EQ:
    /* ---  3バンド・イコライザ --- */
    set_h_LPF(h1, hsize, B1);
    set_h_BPF(h2, hsize, B1, B2);
    set_h_HPF(h3, hsize, B2);
    clear_h(h, hsize);
    muladd_h(h, h, h1, K1, hsize); /* 低域: K1倍 */
    muladd_h(h, h, h2, K2, hsize); /* 中域: K2倍 */
    muladd_h(h, h, h3, K3, hsize); /* 高域: K3倍 */
    break;

  default:
    fprintf(stderr, "error: unknown type number, type = %d\n", type);
    exit(2);
  }
}

/* 窓関数を乗じる */
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

/* インパルス応答を求める */
void h_calc(int type, double *h, int hsize, int win)
{
  if (hsize > MAXHSIZE) {
    fprintf(stderr, "error: too large hsize\n");
    exit(2);
  }

  calc_response(type, h, hsize);
  mult_window(win, h, hsize);
}

/* === FILTER PROGRAM ================================================= */

/* --- Generic Convolution ------------------------------------------ */

void check_usage(int type, int c, char *paramlist, int paramc, char **paramv)
{
  if (c != paramc) {
    fprintf(stderr, "usage: filter <src_fname> <out_fname> %s %s\n", typelist[type], paramlist);
    exit(2);
  }
}

/* インパルス応答の設定 */
int set_impulse_response(double *h, int type, int paramc, char **paramv)
{
  int i;
  int hsize = 0, win;

  switch (type) {
  case TYPE_MEAN:
    /* 移動平均フィルタの係数 */
    check_usage(type, 1, "<#taps>", paramc, paramv);
    hsize = atoi(paramv[0]);
    for (i = 0; i < hsize; i++) {
      h[i] = 1.0 / hsize;
    }
    break;
  case TYPE_LPF:
  case TYPE_HPF:
    check_usage(type, 3, "<#taps> <BW> {rect,hamming,hanning,blackman}", paramc, paramv);
    hsize = atoi(paramv[0]);
    BW = atof(paramv[1]);
    win = scan_win(paramv[2]);
    h_calc(type, h, hsize, win);
    break;
  case TYPE_BPF:
  case TYPE_BRF:
    check_usage(type, 4, "<#taps> <B1> <B2> {rect,hamming,hanning,blackman}", paramc, paramv);
    hsize = atoi(paramv[0]);
    B1 = atof(paramv[1]);
    B2 = atof(paramv[2]);
    win = scan_win(paramv[3]);
    h_calc(type, h, hsize, win);
    break;
  case TYPE_DIFF:
    /* 差分フィルタの係数 */
    check_usage(type, 0, "", paramc, paramv);
    hsize = 2;
    h[0] = 1;
    h[1] = -1;
    break;
  case TYPE_ECHO:
    /* --- never used !!! --- */
    /* ECHO (delay=25ms) の係数 */
    check_usage(type, 0, "", paramc, paramv);
    hsize = 1000;
    for (i = 0; i < 1000; i++) {
      h[i] = 0;
    }
    h[0] = 0.8;
    h[699] = 0.4;
    h[999] = 0.2;
    break;
  case TYPE_EQ:
    /* 5バンド・イコライザ (低域 1.0倍, 中域 0.4倍, 高域 0.8倍)*/
    check_usage(type, 7, "<#taps> <B1> <B2> <k1> <k2> <k3> {rect,hamming,hanning,blackman}", paramc, paramv);
    hsize = atoi(paramv[0]);
    B1 = atof(paramv[1]);
    B2 = atof(paramv[2]);
    K1 = atof(paramv[3]);
    K2 = atof(paramv[4]);
    K3 = atof(paramv[5]);
    win = scan_win(paramv[6]);
    h_calc(type, h, hsize, win);
    break;
  default:
    fprintf(stderr, "error: unknown response type\n");
    exit(2);
  }

  return (hsize);
}

/* エコー処理 */
void do_echo(struct waveform_s *x, struct waveform_s *y, int paramc, char **paramv)
{
  int i, k, m, ch;
  int count;
  double sum;

  int delaycount;
  static double level[MAXHSIZE];
  static int delay[MAXHSIZE];

  /* 係数の初期設定と正規化 */
  delay[0] = 0;
  level[0] = 1.0;
  delaycount = 1;
  sum = 1.0;
  for (i = 0; i + 1 < paramc; i += 2) {
    delay[delaycount] = atoi(paramv[i]);
    level[delaycount] = atof(paramv[i + 1]);
    sum += level[delaycount];
    delaycount++;
  }
  for (k = 0; k < delaycount; k++) {
    level[k] /= sum;
  }

  /* 畳み込み積分 */
  count = x->datasize / sizeof(WAVE_T);

  for (ch = 0; ch < 2; ch++) { /* ch=0: L-ch, ch=1: R-ch */
    for (i = 0; i < count; i++) {
      /* 積分値の初期化 */
      sum = 0.0;
      /* 積分計算 */
      for (k = 0; k < delaycount; k++) {
	m = i - delay[k];
	if (m >= 0 && m < count) {
	  sum += level[k] * (double)x->data[m].ch[ch];
	}
      }
#if 0
      /* ギプス現象による音割れを緩和する */
      // あらかじめ15bit程度のサウンドレベルにしておけば不要
      sum *= 0.5;
#endif
#if 1
      /* クリッピング処理 */
      if (sum >= 32767.0) sum = 32767.0;
      else if (sum < -32767.0) sum = -32767.0;
#endif
      /* 計算結果の代入 */
      y->data[i].ch[ch] = (CH_T)sum;
    }
  }
}

/* 周波数特性の表示 */
void show_ftoku(double *h, int hsize, FILE *out)
{
  int i;
  double wT;
  double re, im, amp;

  fprintf(out, "TitleText: Frequency Response\n");
  fprintf(out, "XUnitText: f / fs\n");
  fprintf(out, "YUnitText: H(f)\n");
  fprintf(out, "Markers: on\n");
  // fprintf(out, "LogY: on\n");
  // fprintf(out, "YLowLimit: -8\n");
  // fprintf(out, "YHighLimit: 0\n");
  fprintf(out, "XLowLimit: -0.5\n");
  fprintf(out, "XHighLimit: 0.5\n");
  fprintf(out, "BoundBox: on\n");

  for (wT = -M_PI; wT <= M_PI; wT += 0.01) {
    re = 0.0;
    im = 0.0;
    for (i = 0; i < hsize; i++) {
      re += h[i] * cos(i * wT);
      im += h[i] * sin(i * wT);
    }
    amp = sqrt(re * re + im * im);
    if (opt_c) {
      amp = pow(amp, opt_c);
    }
    fprintf(out, "%f %e\n", wT / (2.0 * M_PI), amp + DELTA);
  }

  fprintf(out, "\n");
  fflush(out);
}

/* インパルス応答の表示 */
void show_h(double *h, int hsize, FILE *out)
{
  int k;

  for (k = 0; k < hsize; k++) {
    fprintf(out, "%d %e\n", k, h[k]);
  }

  fprintf(out, "\n");
  fflush(out);
}


/* フィルタ処理 */
void do_filter(struct waveform_s *x, struct waveform_s *y, int type, int paramc, char **paramv)
{
  int i, k, m, ch;
  int count;
  double sum;
  double *wavebuf[2];
  int n, cascade;

  int hsize;
  static double h[MAXHSIZE];

  /* 係数の初期設定 */
  hsize = set_impulse_response(h, type, paramc, paramv);

  /* 周波数特性／インパルス応答の表示 */
  if (opt_ftoku) {
    show_ftoku(h, hsize, stdout);
  }
  if (opt_h) {
    show_h(h, hsize, stdout);
  }
  if (opt_z) {
    return;
  }

  /* 畳み込み積分の初期設定 */
  count = x->datasize / sizeof(WAVE_T);
  for (ch = 0; ch < 2; ch++) {
    wavebuf[ch] = m_alloc(sizeof(double) * count);
  }
  if (opt_c) {
    cascade = opt_c;
  } else {
    cascade = 1;
  }
  
  /* 畳み込み積分 */
  for (n = 0; n < cascade; n++) {
    for (ch = 0; ch < 2; ch++) { /* ch=0: L-ch, ch=1: R-ch */
      for (i = 0; i < count; i++) {
	/* 積分値の初期化 */
	sum = 0.0;
	/* 積分計算 */
	for (k = 0; k < hsize; k++) {
	  m = i - k;
	  if (m >= 0 && m < count) {
	    sum += h[k] * (double)x->data[m].ch[ch];
	  }
	}
	wavebuf[ch][i] = sum;
      }
    }
  }

  for (ch = 0; ch < 2; ch++) {
    for (i = 0; i < count; i++) {
#if 0
      /* ギプス現象による音割れを緩和する */
      // あらかじめ15bit程度のサウンドレベルにしておけば不要
      wavebuf[ch][i] *= 0.5;
#endif
#if 1
      /* クリッピング処理 */
      if (wavebuf[ch][i] >= 32767.0) wavebuf[ch][i] = 32767.0;
      else if (wavebuf[ch][i] < -32767.0) wavebuf[ch][i] = -32767.0;
#endif
      /* 計算結果の代入 */
      y->data[i].ch[ch] = (CH_T)wavebuf[ch][i];
    }
  }
}

/* --- MAIN PROGRAM ------------------------------------------------- */

/* src_fname と h(t) を畳み込み積分し，out_fname に出力する */
void filter(char *src_fname, char *out_fname, int type, int paramc, char **paramv)
{
  struct waveform_s wf_x, wf_y;

  /* 波形 x(t) の読み込み */
  load_waveform(src_fname, &wf_x);

#if 0
  /* データサイズが大きいとき，途中までの処理とする */
  if (wf_x.datasize > 3000000) wf_x.datasize = 3000000;
#endif

  /* 波形 y(t) の初期化 */
  wf_y = wf_x;
  wf_y.data = m_alloc(wf_x.datasize);

  /* 波形 y(t) の計算 (畳み込み積分) */
  if (type == TYPE_ECHO) {
    do_echo(&wf_x, &wf_y, paramc, paramv);
  } else {
    do_filter(&wf_x, &wf_y, type, paramc, paramv);
  }

  if (opt_z) {
    return;
  }

  /* 波形 y(t) の保存 */
  save_waveform(out_fname, &wf_y);
}

void usage(void)
{
  fprintf(stderr, "usage: filter [-ftoku] [-h] [-c<#cascades>] [-z] <src_fname> <out_fname> <type> <params...>\n");
  fprintf(stderr, "   ex: filter sweep.wav output.wav 64 lpf 0.1\n");
  // fprintf(stderr, "   ex: filter -ftoku -z sweep.wav output.wav mean 32 | xgraph -lny -ly -3,0\n");
  fprintf(stderr, "type=mean  : 移動平均フィルタ\n");
  fprintf(stderr, "type=lpf   : ローパスフィルタ\n");
  fprintf(stderr, "type=hpf   : ハイパスフィルタ\n");
  fprintf(stderr, "type=bpf   : バンドパスフィルタ\n");
  fprintf(stderr, "type=brf   : バンドリジェクションフィルタ\n");
  fprintf(stderr, "type=diff  : 差分フィルタ\n");
  fprintf(stderr, "type=eq    : 3バンドイコライザ\n");
  fprintf(stderr, "type=echo  : ECHO\n");
  exit(2);
}

void options(int *argc, char ***argv)
{
  char *v;

  while (*argc >= 2) {
    v = *(*argv + 1);
    if (*v != '-') break;
    if (strcasecmp(v, "-ftoku") == 0) {
      opt_ftoku++;
    } else if (strcasecmp(v, "-h") == 0) {
      opt_h++;
    } else if (strcasecmp(v, "-z") == 0) {
      opt_z++;
    } else if (strncasecmp(v, "-c", 2) == 0) {
      opt_c = atoi(v + 2);
    } else {
      fprintf(stderr, "warning: unknown option '%s', ignored\n", v);
    }
    (*argc)--;
    (*argv)++;
  }
}

int main(int argc, char **argv)
{
  char *src_fname, *out_fname;
  int type;

  options(&argc, &argv);
  if (argc < 4) {
    usage();
    exit(2);
  }

  src_fname = *++argv;
  out_fname = *++argv;

  type = scan_type(*++argv);
  if (type < 0) {
    usage();
    exit(2);
  }

  filter(src_fname, out_fname, type, argc - 4, ++argv);

  return (0);
}

