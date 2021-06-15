/* --------------------------------------------------- */
/* RIFFファイル入出力サブルーチン                      */
/* Written by 辻岡哲夫                                 */
/* Powered by 情報通信領域, Osaka City University      */
/* --------------------------------------------------- */

/* 2010.01.12 (Tue) */
/* 2014.01.13 (Tue) revised */
/* 2016.12.05 (Mon) re-written for 96kHz sampling */

/* 100% Original */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* --- DEFINITIONS -------------------------------------------------- */

#define WAVE_T  struct data_t
#define CH_T    int32_t // Cygwin64以降はsizeof(long)は8となったためint32_tに変更した
#define NCHANNELS 2

#pragma pack(2)
struct header_s {
  int16_t type;
  int16_t stereo;
  int32_t playfreq;
  int32_t playspeed;
  int16_t datasize;
  int16_t databits;
  int16_t extsize;
};

struct data_t {
  CH_T ch[NCHANNELS];
};

struct waveform_s {
  struct header_s header;
  char *databuf;
  long databufsize;
  struct data_t *data;
  long datasize;
  long datacount;
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

struct chunk_s {
  char name[5];
  long size;
  char *buf;
};

/* --- LOAD --- */

struct chunk_s chunk_readfile(char *fname)
{
  struct chunk_s chunk;
  FILE *in;

  in = fopen(fname, "rb");
  if (!in) {
    fprintf(stderr, "error: file cannot be open\n");
    exit(2);
  }

  memset(&chunk, 0, sizeof(chunk));
  fread(chunk.name, 1, 4, in);
  fread(&chunk.size, 1, 4, in);
  chunk.buf = m_alloc(chunk.size);
  fread(chunk.buf, 1, chunk.size, in);

  fclose(in);

  return (chunk);
}

struct chunk_s chunk_readbuf(char *buf)
{
  struct chunk_s chunk;

  memset(&chunk, 0, sizeof(chunk));
  memcpy(chunk.name, buf, 4);
  memcpy(&chunk.size, buf + 4, 4);
  chunk.buf = m_alloc(chunk.size);
  memcpy(chunk.buf, buf + 8, chunk.size);

  return (chunk);
}

void load_waveform(char *fname, struct waveform_s *w)
{
  int i;
  char *p;
  unsigned char *u;
  char tmpbuf[4];
  struct chunk_s riff, chunk;
  long remain;

  riff = chunk_readfile(fname);
  p = riff.buf + 4; // skip "WAVE"
  remain = riff.size - 4; // skip "WAVE"
  while (remain > 0) {
    chunk = chunk_readbuf(p);
    // printf("chunk: '%s', ofs: %06x size: %d, remain: %d\n", chunk.name, p - riff.buf, chunk.size, remain);
    p += (8 + chunk.size);
    remain -= (8 + chunk.size);

    if (strcmp(chunk.name, "fmt ") == 0) {
      memcpy(&w->header, chunk.buf, chunk.size);
    } else if (strcmp(chunk.name, "data") == 0) {
      w->databuf = chunk.buf;
      w->databufsize = chunk.size;
      w->datacount = chunk.size / w->header.datasize;
      w->data = m_alloc(sizeof(WAVE_T) * w->datacount);
      w->datasize = sizeof(WAVE_T) * w->datacount;
      // printf("databufsize = %d, datacount = %d, datasize = %d\n", w->databufsize, w->datacount, w->datasize);
    }
    // ignore "LIST" => group("INFO" => subchunk("INAM", "IART", "ICMT"))
    // ignore "JUNK"
  }

  if (w->header.stereo == 2 && (w->header.databits == 16 || w->header.databits == 24)) {
    u = w->databuf;
    for (i = 0; i < w->datacount; i++) {
      if (w->header.databits == 16) {
        w->data[i].ch[0] = (short)(u[0] + u[1] * 256);
        w->data[i].ch[1] = (short)(u[2] + u[3] * 256);
	// if (i < 50) fprintf(stderr, "u: %d %d %d %d, ch: %d %d\n", u[0], u[1], u[2], u[3], w->data[i].ch[0], w->data[i].ch[1]);
      } else {
        w->data[i].ch[0] = (CH_T)(u[0] + u[1] * 256L + u[2] * 65536L);
        w->data[i].ch[1] = (CH_T)(u[3] + u[4] * 256L + u[5] * 65536L);
        if (u[2] >= 128) w->data[i].ch[0] |= 0xff000000;
        if (u[5] >= 128) w->data[i].ch[1] |= 0xff000000;
        // fprintf(stderr, "%d: %ld %ld  +%06x (%02x %02x %02x %02x %02x %02x)\n", i, w->data[i].ch[0], w->data[i].ch[1], (char *)u - w->databuf, u[0], u[1], u[2], u[3], u[4], u[5]);
      }
      u += w->header.datasize;
    }
  } else {
    fprintf(stderr, "error: neither 16bit/stereo nor 24bit/stereo file\n");
    exit(2);
  }
}

/* --- SAVE --- */

void set_header_default(struct waveform_s *w, int fs, int databits)
{
  struct header_s *h;

  h = &w->header;
  h->type = 1;   /* 1: PCM */
  h->stereo = 2; /* 1: mono, 2: stereo */
  h->playfreq  = fs;     /* fs [Hz] */
  if (databits == 16) {
    h->playspeed = fs * 4; /* [bytes/s] */
    h->datasize = 4;  /* L+R=2ch x 16-bit */
    h->databits = 16; /* 16-bit */
  } else if (databits == 24) {
    h->playspeed = fs * 6; /* [bytes/s] */
    h->datasize = 6;  /* L+R=2ch x 24-bit */
    h->databits = 24; /* 24-bit */
  } else {
    fprintf(stderr, "error: unknown databits of %d\n", databits);
    exit(2);
  }
  h->extsize = 0;
}

void save_waveform(char *fname, struct waveform_s *w)
{
  int i;
  char tmpbuf[6];
  int filesize, headersize, datasize;
  FILE *out;

  out = fopen(fname, "wb");
  if (!out) {
    fprintf(stderr, "error: file cannot be created\n");
    exit(2);
  }

  /* RIFF */
  fwrite("RIFF", 1, 4, out);
  datasize = w->datacount * w->header.datasize; // header.datasize = 4 (16-bit) or 6 (24-bit)
  filesize = datasize + 36;
  fwrite(&filesize, 1, 4, out);
  fwrite("WAVE", 1, 4, out);

  /* FMT chunk */
  fwrite("fmt ", 1, 4, out);
  headersize = 16; /* sizeof(w->header) */
  fwrite(&headersize, 1, 4, out);
  fwrite(&w->header, 1, headersize, out);

  /* DATA chunk */
  fwrite("data", 4, 1, out);
  fwrite(&datasize, 1, 4, out);
  for (i = 0; i < w->datacount; i++) {
    if (w->header.databits == 16) {
      memcpy(tmpbuf, &w->data[i].ch[0], 2);
      memcpy(tmpbuf + 2, &w->data[i].ch[1], 2);
      // *((short *)tmpbuf) = (short)w->data[i].ch[0];
      // *((short *)tmpbuf + 1) = (short)w->data[i].ch[1];
      fwrite(tmpbuf, 1, 4, out);
    } else if (w->header.databits == 24) {
      memcpy(tmpbuf, &w->data[i].ch[0], 3);
      memcpy(tmpbuf + 3, &w->data[i].ch[1], 3);
      fwrite(tmpbuf, 1, 6, out);
    }
  }
  fclose(out);
}

/* --- END OF SUB-ROUTINES ------------------------------------------ */
