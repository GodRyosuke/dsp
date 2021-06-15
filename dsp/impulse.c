/* --------------------------------------------------- */
/* インパルス波のサウンドデータ作成プログラム          */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

/* 2001.12.13 (Thu) */
/* 2010.01.12 (Tue) revised */
/* 2014.01.13 (Tue) revised */

/* RIFFサブルーチンの読み込み */
#include "riff.c"

/* --- MAIN PROGRAM ------------------------------------------------- */

/* impulse波形ファイル imp.wav の作成 */
void impulse(char *out_fname, double fs, double sec)
{
  struct waveform_s wf;
  int i, ch;
  int size, size_2;

  /* データ長 sec[秒] */
  size = (int)(fs * sec);

  set_header_default(&wf, (int)fs);
  wf.data = m_alloc(size * sizeof(WAVE_T));
  wf.datasize = size * sizeof(WAVE_T);

  size_2 = size / 2;

  /* L-channel */
  ch = 0;
  for (i = 0; i < size; i++) {
    if (i == size_2) wf.data[i].ch[ch] = (CH_T)10000;
    else             wf.data[i].ch[ch] = (CH_T)0;
  }

  /* R-channel */
  ch = 1;
  for (i = 0; i < size; i++) {
    if (i == size_2) wf.data[i].ch[ch] = (CH_T)10000;
    else             wf.data[i].ch[ch] = (CH_T)0;
  }

  save_waveform(out_fname, &wf);
}

int main(int argc, char **argv)
{
  char *fname;
  double fs, sec;

  if (argc != 3) {
    fprintf(stderr, "usage: impulse <out_fname> <sec>\n");
    fprintf(stderr, "   ex: impulse imp.wav 10\n");
    exit(2);
  }

  fname = argv[1];
  sec = atof(argv[2]);

  /* サンプリング周波数 fs = 11.05 [kHz] */
  fs = 11050;
  // fs = 44100;

  impulse(fname, fs, sec);

  return (0);
}
