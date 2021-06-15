/* --------------------------------------------------- */
/* WAV波形の表示プログラム                             */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

/* 2001.12.17 (Thu) */
/* 2010.01.12 (Tue) revised */
/* 2014.01.13 (Tue) revised */
/* 2016.12.05 (Mon) re-written for 96kHz sampling */

/* RIFFサブルーチンの読み込み */
#include "riff.c"

/* --- MAIN PROGRAM ------------------------------------------------- */

/* src_fname の内容を表示する */
void show_waveform(char *src_fname, int start, int size, int step)
{
  struct waveform_s wf_x;
  int i, ch;
  int count, end;

  /* 波形 x(t) の読み込み */
  load_waveform(src_fname, &wf_x);
  fprintf(stderr, "fname = %s, datacount = %d, databits = %d, freq = %.1fkHz, time = %.3f\n", src_fname, wf_x.datacount, wf_x.header.databits, (double)wf_x.header.playfreq / 1000.0, (double)wf_x.datacount / wf_x.header.playfreq);

  /* get data count */
  count = wf_x.datasize / sizeof(WAVE_T);

  /* if minus, set tail region */
  if (start < 0) {
    start = count + start;
  }

  /* get the end position to show*/
  end = start + size;
  if (end > count) end = count;

  /* x(t)の表示 */
  for (i = start; i < end; i += step) {
    printf("%d %ld %ld\n", i, (long)wf_x.data[i].ch[0], (long)wf_x.data[i].ch[1]);
  }

  printf("\n"); /* 空白行挿入 */
}

int main(int argc, char **argv)
{
  char *src_fname;
  int start, size, step;

  if (argc < 4) {
    fprintf(stderr, "usage: show <src_fname> <start> <size> [<step>]\n");
    fprintf(stderr, "   ex: show data.wav 0 1000\n");
    exit(2);
  }

  src_fname = argv[1];
  start = atoi(argv[2]);
  size  = atoi(argv[3]);
  if (argc >= 5) step = atoi(argv[4]);
  else           step = 1;

  show_waveform(src_fname, start, size, step);

  return (0);
}

