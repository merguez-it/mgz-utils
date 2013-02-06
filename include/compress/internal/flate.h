#ifndef __MGZ_COMPRESS_INTERNALFLATE_H
#define __MGZ_COMPRESS_INTERNALFLATE_H

/* deflate and inflate return values */
enum {
	FLATE_OK  = 0,
	FLATE_ERR = -1,
	FLATE_IN  = -2,
	FLATE_OUT = -3
};

typedef struct {
	int avail_in;
	int avail_out;
	unsigned char *next_in;
	unsigned char *next_out;
	char *err;
	void *state;
} flat_stream;

#ifdef __cplusplus
extern "C" {
#endif

int deflate(flat_stream *s);
int inflate(flat_stream *s);

#ifdef __cplusplus
}
#endif

#endif // __MGZ_COMPRESS_INTERNALFLATE_H
