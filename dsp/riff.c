/* --------------------------------------------------- */
/* RIFFファイル入出力サブルーチン                      */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

/* 2010.01.12 (Tue) */
/* 2014.01.13 (Tue) revised */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* --- DEFINITIONS -------------------------------------------------- */

#define WAVE_T  struct data_t
#define CH_T    short
#define NCHANNELS 2

#pragma pack(2)
struct header_s {
  short type;
  short stereo;
  int   playfreq;
  int   playspeed;
  short datasize;
  short databits;
  short extsize;
};

struct data_t {
  CH_T ch[NCHANNELS];
};

struct waveform_s {
  struct header_s header;
  struct data_t *data;
  int datasize;
};

void *m_alloc(int size)
{
  void *p;

  p = malloc(size);
  if (p == NULL) {
    fprintf(stderr, "error: not enough memory\n");
    exit(2);
  }

  return (p);
}

/* --- WAV RIFF ----------------------------------------------------- */

void set_header_default(struct waveform_s *w, int fs)
{
  struct header_s *h;

  h = &w->header;
  h->type = 1;   /* 1: PCM */
  h->stereo = 2; /* 1: mono, 2: stereo */
  h->playfreq  = fs;     /* fs [Hz] */
  h->playspeed = fs * 4; /* [bytes/s] */
  h->datasize = 4;  /* L+R=2ch x 16-bit */
  h->databits = 16; /* 16-bit */
  h->extsize = 0;
}

void load_waveform(char *fname, struct waveform_s *w)
{
  char tmpbuf[4];
  int filesize, headersize;
  FILE *in;

  in = fopen(fname, "rb");
  if (!in) {
    fprintf(stderr, "error: file cannot be open\n");
    exit(2);
  }

  /* RIFF */
  fread(tmpbuf, 1, 4, in); /* "RIFF" */
  fread(&filesize, 1, 4, in);
  fread(tmpbuf, 1, 4, in); /* "WAVE" */

  /* FMT chunk */
  fread(tmpbuf, 1, 4, in); /* "fmt " */
  fread(&headersize, 1, 4, in);
  fread(&w->header, 1, headersize, in);

  /* DATA chunk */
  fread(tmpbuf, 4, 1, in); /* "data" */
  fread(&w->datasize, 1, 4, in);
  w->data = m_alloc(w->datasize);
  fread(w->data, 1, w->datasize, in);

  if (w->header.stereo != 2 || w->header.databits != 16) {
    fprintf(stderr, "error: not a 16bit/stereo file\n");
    exit(2);
  }

  fclose(in);
}

void save_waveform(char *fname, struct waveform_s *w)
{
  int filesize, headersize;
  FILE *out;

  out = fopen(fname, "wb");
  if (!out) {
    fprintf(stderr, "error: file cannot be created\n");
    exit(2);
  }

  /* RIFF */
  fwrite("RIFF", 1, 4, out);
  filesize = w->datasize + 36;
  fwrite(&filesize, 1, 4, out);
  fwrite("WAVE", 1, 4, out);

  /* FMT chunk */
  fwrite("fmt ", 1, 4, out);
  headersize = 16; /* sizeof(w->header) */
  fwrite(&headersize, 1, 4, out);
  fwrite(&w->header, 1, headersize, out);

  /* DATA chunk */
  fwrite("data", 4, 1, out);
  fwrite(&w->datasize, 1, 4, out);
  fwrite(w->data, 1, w->datasize, out);

  fclose(out);
}

/* --- END OF SUB-ROUTINES ------------------------------------------ */

