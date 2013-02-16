#ifndef __Z_STREAM_H
#define __Z_STREAM_H

enum {
	FLATE_OK  = 0,
	FLATE_ERR = -1,
	FLATE_IN  = -2,
	FLATE_OUT = -3,
  FLATE_END  = -4
};

typedef struct {
	int avail_in;
	int avail_out;
	unsigned char *next_in;
	unsigned char *next_out;
  unsigned char *begin;
  unsigned char inflate_header_read;
	char *err;
	void *state;
} mgz_stream;

#endif // __Z_STREAM_H

