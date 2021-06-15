/* --------------------------------------------------- */
/* 指定されたS/Nで雑音を加えるプログラム               */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

/* 2006.05.19 (Fri) */
/* 2010.01.12 (Tue) revised */
/* 2014.01.13 (Tue) revised */

/* RIFFサブルーチンの読み込み */
#include "riff.c"

#define drandom() (random() / 2147483648.0)

/* 平均0,分散1の正規分布乱数を生成する */
double normal_random(void)
{
  int i;
  double r = 0;

  for (i = 0; i < 12; i++) {
    r += drandom();
  }
  return (r - 6.0);
}

/* 信号電力の計算 */
double measure_power(struct waveform_s *x)
{
  int i, ch;
  int count;
  double sum[2], signal_power;

  count = x->datasize / sizeof(WAVE_T);

  for (ch = 0; ch < 2; ch++) { /* ch=0: L-ch, ch=1: R-ch */
    sum[ch] = 0.0;
    for (i = 0; i < count; i++) {
      /* 電力を積算する */
      sum[ch] += (double)x->data[i].ch[ch] * (double)x->data[i].ch[ch];
    }
  }

  signal_power = (sum[0] + sum[1]) / (count * 2);

  return (signal_power);
}

/* フィルタ処理 */
void do_addnoise(struct waveform_s *x, struct waveform_s *y, double sigma)
{
  int i, ch;
  int count;
  double signal, noise;

  count = x->datasize / sizeof(WAVE_T);

  for (ch = 0; ch < 2; ch++) { /* ch=0: L-ch, ch=1: R-ch */
    for (i = 0; i < count; i++) {
      /* 雑音の加算 */
      signal = (double)x->data[i].ch[ch];
      noise = normal_random() * sigma;
      signal += noise;

      /* クリッピング処理 */
      if (signal >= 32767.0) signal = 32767.0;
      else if (signal < -32767.0) signal = -32767.0;

      /* 計算結果の代入 */
      y->data[i].ch[ch] = (CH_T)signal;
    }
  }
}

/* --- MAIN PROGRAM ------------------------------------------------- */

/* src_fname に雑音を加算し，out_fname に出力する */
void addnoise(char *src_fname, char *out_fname, double SNR)
{
  struct waveform_s wf_x, wf_y;
  double signal_power, sigma2, sigma;

  /* 波形 x(t) の読み込み */
  load_waveform(src_fname, &wf_x);

#if 0
  /* データサイズが大きいとき，途中までの処理とする */
  if (wf_x.datasize > 3000000) wf_x.datasize = 3000000;
#endif

  /* 波形 y(t) の初期化 */
  wf_y = wf_x;
  wf_y.data = m_alloc(wf_x.datasize);

  /* 電力を計算する */
  signal_power = measure_power(&wf_x);
  fprintf(stderr, "signal_power = %f\n", signal_power);

  sigma2 = signal_power * pow(10.0, -SNR / 10.0);
  sigma = sqrt(sigma2);

  /* ノイズを加算する */
  do_addnoise(&wf_x, &wf_y, sigma);

  /* 波形 y(t) の保存 */
  save_waveform(out_fname, &wf_y);
}

int main(int argc, char **argv)
{
  double SNR;
  char *src_fname, *out_fname;

  if (argc != 4) {
    fprintf(stderr, "usage: addnoise <src_fname> <out_fname> <SNR>\n");
    exit(2);
  }

  src_fname = *++argv;
  out_fname = *++argv;
  SNR = atof(*++argv);

  addnoise(src_fname, out_fname, SNR);

  return (0);
}

