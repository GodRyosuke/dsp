/* --------------------------------------------------- */
/* サイン波のサウンドデータ作成プログラム              */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

/* 2010.01.12 (Tue) */
/* 2014.01.13 (Tue) revised */

/* RIFFサブルーチンの読み込み */
#include "riff.c"

/* --- MAIN PROGRAM ------------------------------------------------- */

/* sin波形ファイル sin.wav の作成 */
void sinwave(char *out_fname, double fs, double f, double sec, double ofs)
{
  struct waveform_s wf;
  int i, ch;
  int size;

  /* データ長 sec[秒] */
  size = (int)(fs * sec);

  set_header_default(&wf, (int)fs);
  wf.data = m_alloc(size * sizeof(WAVE_T));
  wf.datasize = size * sizeof(WAVE_T);

  /* L-channel */
  ch = 0;
  for (i = 0; i < size; i++) {
    wf.data[i].ch[ch] = (CH_T)(sin(i / (fs / f) * 2*M_PI + ofs * 2*M_PI) * 10000);
  }

  /* R-channel */
  ch = 1;
  for (i = 0; i < size; i++) {
    wf.data[i].ch[ch] = (CH_T)(sin(i / (fs / f) * 2*M_PI + ofs * 2*M_PI) * 10000);
  }

  save_waveform(out_fname, &wf);
}

int main(int argc, char **argv)
{
  char *fname;
  double fs, f, sec, ofs;

  if (argc != 4 && argc != 5) {
    fprintf(stderr, "usage: sinwave <out_fname> <freq> <sec> [<ofs>]\n");
    fprintf(stderr, "   ex: sinwave sin.wav 440 10\n");
    fprintf(stderr, "   ex: sinwave sin.wav 440 10 0.5  : -sin wt\n");
    exit(2);
  }

  fname = argv[1];
  f   = atof(argv[2]);
  sec = atof(argv[3]);
  if (argc == 5) {
    ofs = atof(argv[4]);
  } else {
    ofs = 0.0;
  }

  /* サンプリング周波数 fs = 44.10 [kHz] */
  // fs = 11050;
  fs = 44100;

  sinwave(fname, fs, f, sec, ofs);

  return (0);
}
