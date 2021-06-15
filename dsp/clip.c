/* --------------------------------------------------- */
/* WAVファイルの一部切出し変換プログラム               */
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
void clip(char *src_fname, char *out_fname, int start, int length)
{
  int i;
  struct waveform_s wf_x, wf_y;
  int count;
  int k;

  // k = 1; /* 16bitのままの時 */
  k = 2; /* 16bit→15bitに落とす時 */

  /* 波形 x(t) の読み込み */
  load_waveform(src_fname, &wf_x);
  count = wf_x.datasize / sizeof(WAVE_T);

  /* 波形 y(t) の初期化 */
  if (start + length > count) {
    length = count - start;
    if (length < 0) {
      length = 0;
    }
  }
  wf_y = wf_x;
  wf_y.datasize = length * sizeof(WAVE_T);
  wf_y.data = m_alloc(wf_y.datasize);

  /* 波形 y(t) の計算 */
  for (i = 0; i < length; i++) {
    wf_y.data[i].ch[0] = wf_x.data[i + start].ch[0] / k;
    wf_y.data[i].ch[1] = wf_x.data[i + start].ch[1] / k;
  }

  printf("fs = %f [ksps]\n", (double)wf_y.header.playfreq / 1000.0);
  printf("original size = %d\n", count);
  printf("start = %d, length = %d\n", start, length);
  printf("quantum division value k = %d\n", k);

  /* 波形 y(t) の保存 */
  save_waveform(out_fname, &wf_y);
}

int main(int argc, char **argv)
{
  char *src_fname, *out_fname;
  int start, length;

  if (argc != 5) {
    fprintf(stderr, "usage: clip <src_fname> <out_fname> <start> <length>\n");
    fprintf(stderr, "   ex: clip input.wav output.wav 0 10000\n");
    fprintf(stderr, "   start and length must be integer numbers\n");
    exit(2);
  }

  src_fname = argv[1];
  out_fname = argv[2];
  start = atoi(argv[3]);
  length = atoi(argv[4]);

  clip(src_fname, out_fname, start, length);

  return (0);
}
