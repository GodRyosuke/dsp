/* --------------------------------------------------- */
/* WAV波形の表示プログラム (L-channelのみ)             */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

/* 2001.12.17 (Thu) */
/* 2010.01.12 (Tue) revised */
/* 2014.01.13 (Tue) revised */

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

  /* get data count */
  count = wf_x.datasize / sizeof(WAVE_T);

  /* get the end position to show*/
  end = start + size;
  if (end > count) end = count;

  /* x(t)の表示 */
  ch = 0;
  for (i = start; i < end; i += step) {
    printf("%d %d\n", i, wf_x.data[i].ch[ch]);
  }

  /* 空白行 (xgraphのセグメント区切り) */
  printf("\n");
}

int main(int argc, char **argv)
{
  char *src_fname;
  int start, size, step;

  if (argc < 4) {
    fprintf(stderr, "usage: show <src_fname> <start> <size> [<step>]\n");
    fprintf(stderr, "   ex: show output.wav 0 1000\n");
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

