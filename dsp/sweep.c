/* --------------------------------------------------- */
/* スイープ波形のサウンドデータ作成プログラム          */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

/* 2010.01.12 (Tue) */
/* 2014.01.13 (Tue) revised */

/* RIFFサブルーチンの読み込み */
#include "riff.c"

/* --- MAIN PROGRAM ------------------------------------------------- */

// #define LOG_SCALE

/* sweep波形ファイル sweep.wav の作成 */
void sweepwave(char *out_fname, double fs, double f1, double f2, double sec)
{
  struct waveform_s wf;
  int i;
  int size;
  double phase, step, f;

  /* データ長: sec[秒] */
  size = (int)(fs * sec);

  set_header_default(&wf, (int)fs);
  wf.data = m_alloc(size * sizeof(WAVE_T));
  wf.datasize = size * sizeof(WAVE_T);

  /* LR-channel */
  phase = 0.0;
#ifdef LOG_SCALE
  step = pow((double)f2 / f1, 1.0 / (double)size); /* Log */
#else
  step = (f2 - f1) / size;                     /* Linear */
#endif
  for (i = 0; i < size; i++) {
    wf.data[i].ch[0] = (CH_T)(sin(phase) * 10000);
    wf.data[i].ch[1] = (CH_T)(sin(phase) * 10000);
#ifdef LOG_SCALE
    f = pow(step, (double)i) * f1; /* Log */
#else
    f = f1 + (step * i);           /* Linear */
#endif
    phase += 2.0 * M_PI * f / fs;
  }

  save_waveform(out_fname, &wf);
}

int main(int argc, char **argv)
{
  char *fname;
  double f1, f2, sec;
  double fs;

  if (argc != 5) {
    fprintf(stderr, "usage: sweep <out_fname> <f1> <f2> <second>\n");
    fprintf(stderr, "   ex: sweep sweep.wav 10 22050 20\n");
    exit(2);
  }

  fname = argv[1];
  f1  = atof(argv[2]);
  f2  = atof(argv[3]);
  sec = atof(argv[4]);

  /* サンプリング周波数 fs = 11.05 [kHz] */
  // fs = 11050;
  fs = 44100;

  sweepwave(fname, fs, f1, f2, sec);

  return (0);
}
