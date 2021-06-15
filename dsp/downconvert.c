/* --------------------------------------------------- */
/* サンプリング周波数の変換プログラム                  */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

/* 2001.12.13 (Thu) */
/* 2010.01.12 (Tue) revised */
/* 2014.01.13 (Tue) revised */

/* RIFFサブルーチンの読み込み */
#include "riff.c"

/* --- MAIN PROGRAM ------------------------------------------------- */

/* サンプリング周波数変換 → 1/4 の 11.05kHz */
void downconvert(char *src_fname, char *out_fname, int N)
{
  int i;
  struct waveform_s wf_x, wf_y;
  int count;
  int k;

  k = 1; /* 16bitのままの時 */
  // k = 2; /* 16bit→15bitに落とす時 */

  /* 波形 x(t) の読み込み */
  load_waveform(src_fname, &wf_x);

  /* 波形 y(t) の初期化 */
  wf_y = wf_x;
  wf_y.datasize /= N;
  wf_y.data = m_alloc(wf_y.datasize);

  /* 波形 y(t) の計算 */
  count = wf_y.datasize / sizeof(WAVE_T);
  for (i = 0; i < count; i++) {
    wf_y.data[i].ch[0] = wf_x.data[i * N].ch[0] / k;
    wf_y.data[i].ch[1] = wf_x.data[i * N].ch[1] / k;
  }

  printf("original  fs = %f [ksps]\n", (double)wf_y.header.playfreq / 1000.0);
  wf_y.header.playfreq  /= N; /* fs [Hz] */
  wf_y.header.playspeed /= N; /* [bytes/s] */

  printf("converted fs = %f [ksps]\n", (double)wf_y.header.playfreq / 1000.0);

  printf("quantum division value k = %d\n", k);

  /* 波形 y(t) の保存 */
  save_waveform(out_fname, &wf_y);
}

int main(int argc, char **argv)
{
  char *src_fname, *out_fname;
  int N;

  if (argc != 4) {
    fprintf(stderr, "usage: downconvert <src_fname> <out_fname> <N>\n");
    fprintf(stderr, "   ex: downconvert input.wav output.wav 4\n");
    fprintf(stderr, "   N must be an integer number\n");
    exit(2);
  }

  src_fname = argv[1];
  out_fname = argv[2];
  N = atoi(argv[3]);

  downconvert(src_fname, out_fname, N);

  return (0);
}
