/* --------------------------------------------------- */
/* サイン波のサウンドデータ作成プログラム              */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

/* 2010.01.12 (Tue) */
/* 2014.01.13 (Tue) revised */
/* 2016.01.15 (Sun) revised for 96kHz sampling */

/* RIFFサブルーチンの読み込み */
#include "riff.c"

/* --- MAIN PROGRAM ------------------------------------------------- */

/* sin波形ファイル sin.wav の作成 */
void sinwave(char *out_fname, double f, double sec, int fs, int databits, double ofs)
{
  struct waveform_s wf;
  int i;
  int size;
  double amplitude;

  /* データ長 sec[秒] */
  size = (int)(fs * sec);

  set_header_default(&wf, fs, databits);
  wf.data = m_alloc(size * sizeof(WAVE_T));
  wf.datasize = size * sizeof(WAVE_T);
  wf.datacount = size;

  amplitude = 10000.0;
  // amplitude = 20.0;
  if (databits == 24) {
    amplitude *= 256;
  }

  for (i = 0; i < size; i++) {
    wf.data[i].ch[0] = wf.data[i].ch[1] = (CH_T)(sin(i / ((double)fs / f) * 2 * M_PI + ofs * 2 * M_PI) * amplitude);
  }

  save_waveform(out_fname, &wf);
}

int main(int argc, char **argv)
{
  char *fname;
  double f, sec, ofs;
  int fs, databits;

  if (argc < 4 || argc >= 8) {
    fprintf(stderr, "usage: sinwave <out_fname> <freq> <sec> [<freq> [<databits> [<ofs>]]]\n");
    fprintf(stderr, "   ex: sinwave sin.wav 440 10\n");
    fprintf(stderr, "   ex: sinwave sin.wav 440 10 96000 24\n");
    fprintf(stderr, "   ex: sinwave sin.wav 440 10 96000 24 0.5 : -sin wt\n");
    exit(2);
  }

  fname = argv[1];
  f   = atof(argv[2]);
  sec = atof(argv[3]);

  fs = 44100; /* サンプリング周波数 default: fs = 44.1 kHz */
  if (argc >= 5) {
    fs = atoi(argv[4]);
  }

  databits = 16; /* 量子化ビット数 default: 16-bit */
  if (argc >= 6) {
    databits = atoi(argv[5]);
  }

  ofs = 0.0; /* 初期位相 (0～1) default: 0 */
  if (argc >= 7) {
    ofs = atof(argv[6]);
  }

  sinwave(fname, f, sec, fs, databits, ofs);

  return (0);
}
