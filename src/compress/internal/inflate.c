#include <stdlib.h>
#include <string.h>
#include "compress/internal/flate.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	CodeBits        = 16,  /* max number of bits in a code + 1 */
	LitlenTableBits = 9,   /* litlen code bits used in lookup table */
	DistTableBits   = 6,   /* dist code bits used in lookup table */
	ClenTableBits   = 6,   /* clen code bits used in lookup table */
	TableBits       = LitlenTableBits, /* log2(lookup table size) */
	Nlit            = 256, /* number of lit codes */
	Nlen            = 29,  /* number of len codes */
	Nlitlen         = Nlit+Nlen+3, /* litlen codes + block end + 2 unused */
	Ndist           = 30,  /* number of distance codes */
	Nclen           = 19,  /* number of code length codes */
	WinSize         = 1 << 15 /* output window size */
};

/* states */
enum {
	BlockHead,
	UncompressedBlock,
	CopyUncompressed,
	FixedHuff,
	DynamicHuff,
	DynamicHuffClen,
	DynamicHuffLitlenDist,
	DynamicHuffContinue,
	DecodeBlock,
	DecodeBlockLenBits,
	DecodeBlockDist,
	DecodeBlockDistBits,
	DecodeBlockCopy
};

typedef struct {
	short len;  /* code length */
	unsigned short sym; /* symbol */
} Entry;

/* huffman code tree */
typedef struct {
	Entry table[1 << TableBits]; /* prefix lookup table */
	unsigned int nbits;             /* prefix length (table size is 1 << nbits) */
	unsigned int sum;               /* full codes in table: sum(count[0..nbits]) */
	unsigned short count[CodeBits]; /* number of codes with given length */
	unsigned short symbol[Nlitlen]; /* symbols ordered by code length (lexic.) */
} Huff;

typedef struct {
	unsigned char *src;  /* input buffer pointer */
	unsigned char *srcend;

	unsigned int bits;
	unsigned int nbits;

	unsigned char win[WinSize]; /* output window */
	unsigned int pos;    /* window pos */
	unsigned int posout; /* used for flushing win */

	int state;   /* decode state */
	int final;   /* last block flag */
	char *err;   /* TODO: error message */

	/* for decoding dynamic code trees in inflate() */
	int nlit;
	int ndist;
	int nclen;   /* also used in decode_block() */
	int lenpos;  /* also used in decode_block() */
	unsigned char lens[Nlitlen + Ndist];

	int fixed;   /* fixed code tree flag */
	Huff lhuff;  /* dynamic lit/len huffman code tree */
	Huff dhuff;  /* dynamic distance huffman code tree */
} State;

/* TODO: globals.. initialization is not thread safe */
static Huff lhuff; /* fixed lit/len huffman code tree */
static Huff dhuff; /* fixed distance huffman code tree */

/* base offset and extra bits tables */
static unsigned char lenbits[Nlen] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};
static unsigned short lenbase[Nlen] = {
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};
static unsigned char distbits[Ndist] = {
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};
static unsigned short distbase[Ndist] = {
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};

/* ordering of code lengths */
static unsigned char clenorder[Nclen] = {
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

/* TODO: this or normal inc + reverse() */
/* increment bitwise reversed n (msb is bit 0, lsb is bit len-1) */
static unsigned int revinc(unsigned int n, unsigned int len) {
	unsigned int i = 1 << (len - 1);

	while (n & i)
		i >>= 1;
	if (i) {
		n &= i - 1;
		n |= i;
	} else
		n = 0;
	return n;
}

/* build huffman code tree from code lengths (each should be < CodeBits) */
static int build_huff(Huff *huff, unsigned char *lens, unsigned int n, unsigned int nbits) {
	int offs[CodeBits];
	int left;
	unsigned int i, c, sum, code, len, min, max;
	unsigned short *count = huff->count;
	unsigned short *symbol = huff->symbol;
	Entry *table = huff->table;
	Entry entry;

	/* count code lengths */
	for (i = 0; i < CodeBits; i++)
		count[i] = 0;
	for (i = 0; i < n; i++)
		count[lens[i]]++;
	if (count[0] == n) {
		huff->nbits = table[0].len = 0;
		return 0;
	}
	count[0] = 0;

	/* bound code lengths, force nbits to be within the bounds */
	for (max = CodeBits - 1; max > 0; max--)
		if (count[max] != 0)
			break;
	if (nbits > max)
		nbits = max;
	for (min = 1; min < CodeBits; min++)
		if (count[min] != 0)
			break;
	if (nbits < min) {
		nbits = min;
		if (nbits > TableBits)
			return -1;
	}
	huff->nbits = nbits;

	/* check if length is over-subscribed or incomplete */
	for (left = 1 << min, i = min; i <= max; left <<= 1, i++) {
		left -= count[i];
		/* left < 0: over-subscribed, left > 0: incomplete */
		if (left < 0)
			return -1;
	}

	for (sum = 0, i = 0; i <= max; i++) {
		offs[i] = sum;
		sum += count[i];
	}
	/* needed for decoding codes longer than nbits */
	if (nbits < max)
		huff->sum = offs[nbits + 1];

	/* sort symbols by code length (lexicographic order) */
	for (i = 0; i < n; i++)
		if (lens[i])
			symbol[offs[lens[i]]++] = i;

	/* lookup table for decoding nbits from input.. */
	for (i = 0; i < 1 << nbits; i++)
		table[i].len = table[i].sym = 0;
	code = 0;
	/* ..if code is at most nbits (bits are in reverse order, sigh..) */
	for (len = min; len <= nbits; len++)
		for (c = count[len]; c > 0; c--) {
			entry.len = len;
			entry.sym = *symbol;
			for (i = code; i < 1 << nbits; i += 1 << len)
				table[i] = entry;
			/* next code */
			symbol++;
			code = revinc(code, len);
		}
	/* ..if code is longer than nbits: values for simple bitwise decode */
	for (i = 0; code; i++) {
		table[code].len = -1;
		table[code].sym = i << 1;
		code = revinc(code, nbits);
	}
	return 0;
}

/* fixed huffman code trees (should be done at compile time..) */
static void init_fixed_huffs(void) {
	int i;
	unsigned char lens[Nlitlen];

	for (i = 0; i < 144; i++)
		lens[i] = 8;
	for (; i < 256; i++)
		lens[i] = 9;
	for (; i < 280; i++)
		lens[i] = 7;
	for (; i < Nlitlen; i++)
		lens[i] = 8;
	build_huff(&lhuff, lens, Nlitlen, 8);

	for (i = 0; i < Ndist; i++)
		lens[i] = 5;
	build_huff(&dhuff, lens, Ndist, 5);
}

/* fill *bits with n bits from *src */
static int fillbits_fast(unsigned char **src, unsigned char *srcend, unsigned int *bits, unsigned int *nbits, unsigned int n) {
	while (*nbits < n) {
		if (*src == srcend)
			return 0;
		*bits |= *(*src)++ << *nbits;
		*nbits += 8;
	}
	return 1;
}

/* get n bits from *bits */
static unsigned int getbits_fast(unsigned int *bits, unsigned int *nbits, int n) {
	unsigned int k;

	k = *bits & ((1 << n) - 1);
	*bits >>= n;
	*nbits -= n;
	return k;
}

static int fillbits(State *s, unsigned int n) {
	return fillbits_fast(&s->src, s->srcend, &s->bits, &s->nbits, n);
}

static unsigned int getbits(State *s, unsigned int n) {
	return getbits_fast(&s->bits, &s->nbits, n);
}

/* decode symbol bitwise if code is longer than huffbits */
static unsigned int decode_symbol_long(State *s, Huff *huff, unsigned int bits, unsigned int nbits, int cur) {
	int sum = huff->sum;
	unsigned int huffbits = huff->nbits;
	unsigned short *count = huff->count + huffbits + 1;

	/* get bits if we are near the end */
	if (s->src + 2 >= s->srcend) {
		while (nbits < CodeBits - 1 && s->src < s->srcend) {
			bits |= *s->src++ << nbits;
			nbits += 8;
		}
		s->bits = bits;
		s->nbits = nbits;
	}
	bits >>= huffbits;
	nbits -= huffbits;
	for (;;) {
		if (!nbits--) {
			if (s->src == s->srcend)
				return FLATE_IN;
			bits = *s->src++;
			nbits = 7;
		}
		cur |= bits & 1;
		bits >>= 1;
		sum += *count;
		cur -= *count;
		if (cur < 0)
			break;
		cur <<= 1;
		count++;
		if (count == huff->count + CodeBits)
			return s->err = "symbol decoding failed.", FLATE_ERR;
	}
	s->bits = bits;
	s->nbits = nbits;
	return huff->symbol[sum + cur];
}

/* decode a symbol from stream with huff code */
static unsigned int decode_symbol(State *s, Huff *huff) {
	unsigned int huffbits = huff->nbits;
	unsigned int nbits = s->nbits;
	unsigned int bits = s->bits;
	unsigned int mask = (1 << huffbits) - 1;
	Entry entry;

	/* get enough bits efficiently */
	if (nbits < huffbits) {
		unsigned char *src = s->src;

		if (src + 2 < s->srcend) {
			/* we assume huffbits <= 9 */
			bits |= *src++ << nbits;
			nbits += 8;
			bits |= *src++ << nbits;
			nbits += 8;
			bits |= *src++ << nbits;
			nbits += 8;
			s->src = src;
		} else /* rare */
			do {
				if (s->src == s->srcend) {
					entry = huff->table[bits & mask];
					if (entry.len > 0 && entry.len <= nbits) {
						s->bits = bits >> entry.len;
						s->nbits = nbits - entry.len;
						return entry.sym;
					}
					s->bits = bits;
					s->nbits = nbits;
					return FLATE_IN;
				}
				bits |= *s->src++ << nbits;
				nbits += 8;
			} while (nbits < huffbits);
	}
	/* decode bits */
	entry = huff->table[bits & mask];
	if (entry.len > 0) {
		s->bits = bits >> entry.len;
		s->nbits = nbits - entry.len;
		return entry.sym;
	} else if (entry.len == 0)
		return s->err = "symbol decoding failed.", FLATE_ERR;
	return decode_symbol_long(s, huff, bits, nbits, entry.sym);
}

/* decode a block of data from stream with trees */
static int decode_block(State *s, Huff *lhuff, Huff *dhuff) {
	unsigned char *win = s->win;
	unsigned int pos = s->pos;
	unsigned int sym = s->nclen;
	unsigned int len = s->lenpos;
	unsigned int dist = s->nclen;

	switch (s->state) {
	case DecodeBlock:
	for (;;) {
		sym = decode_symbol(s, lhuff);
		if (sym < 256) {
			win[pos++] = sym;
			if (pos == WinSize) {
				s->pos = WinSize;
				s->state = DecodeBlock;
				return FLATE_OUT;
			}
		} else if (sym > 256) {
			sym -= 257;
			if (sym >= Nlen) {
				s->pos = pos;
				s->state = DecodeBlock;
				if (sym + 257 == (unsigned int)FLATE_IN)
					return FLATE_IN;
				return FLATE_ERR;
			}
	case DecodeBlockLenBits:
			if (!fillbits_fast(&s->src, s->srcend, &s->bits, &s->nbits, lenbits[sym])) {
				s->nclen = sym; /* using nclen to store sym */
				s->pos = pos;
				s->state = DecodeBlockLenBits;
				return FLATE_IN;
			}
			len = lenbase[sym] + getbits_fast(&s->bits, &s->nbits, lenbits[sym]);
	case DecodeBlockDist:
			sym = decode_symbol(s, dhuff);
			if (sym == (unsigned int)FLATE_IN) {
				s->pos = pos;
				s->lenpos = len;
				s->state = DecodeBlockDist;
				return FLATE_IN;
			}
			if (sym >= Ndist)
				return FLATE_ERR;
	case DecodeBlockDistBits:
			if (!fillbits_fast(&s->src, s->srcend, &s->bits, &s->nbits, distbits[sym])) {
				s->nclen = sym; /* using nclen to store sym */
				s->pos = pos;
				s->lenpos = len;
				s->state = DecodeBlockDistBits;
				return FLATE_IN;
			}
			dist = distbase[sym] + getbits_fast(&s->bits, &s->nbits, distbits[sym]);
			/* copy match, loop unroll in common case */
			if (pos + len < WinSize) {
				/* lenbase[sym] >= 3 */
				do {
					win[pos] = win[(pos - dist) % WinSize];
					pos++;
					win[pos] = win[(pos - dist) % WinSize];
					pos++;
					win[pos] = win[(pos - dist) % WinSize];
					pos++;
					len -= 3;
				} while (len >= 3);
				if (len--) {
					win[pos] = win[(pos - dist) % WinSize];
					pos++;
					if (len) {
						win[pos] = win[(pos - dist) % WinSize];
						pos++;
					}
				}
			} else { /* rare */
	case DecodeBlockCopy:
				while (len--) {
					win[pos] = win[(pos - dist) % WinSize];
					pos++;
					if (pos == WinSize) {
						s->pos = WinSize;
						s->lenpos = len;
						s->nclen = dist; /* using nclen to store dist */
						s->state = DecodeBlockCopy;
						return FLATE_OUT;
					}
				}
			}
		} else { /* EOB: sym == 256 */
			s->pos = pos;
			return FLATE_OK;
		}
	} /* for (;;) */
	} /* switch () */
	return s->err = "corrupted state.", FLATE_ERR;
}

/* inflate state machine (decodes s->src into s->win) */
static int inflate_state(State *s) {
	int n;
	unsigned int sym;

	if (s->posout)
		return FLATE_OUT;
	for (;;) {
		switch (s->state) {
		case BlockHead:
			if (s->final) {
				if (s->pos)
					return FLATE_OUT;
				else
					return FLATE_OK;
			}
			if (!fillbits(s, 3))
				return FLATE_IN;
			s->final = getbits(s, 1);
			n = getbits(s, 2);
			if (n == 0)
				s->state = UncompressedBlock;
			else if (n == 1)
				s->state = FixedHuff;
			else if (n == 2)
				s->state = DynamicHuff;
			else
				return s->err = "corrupt block header.", FLATE_ERR;
			break;
		case UncompressedBlock:
			/* start block on a byte boundary */
			s->bits >>= s->nbits & 7;
			s->nbits &= ~7;
			if (!fillbits(s, 32))
				return FLATE_IN;
			s->lenpos = getbits(s, 16);
			n = getbits(s, 16);
			if (s->lenpos != (~n & 0xffff))
				return s->err = "corrupt uncompressed length.", FLATE_ERR;
			s->state = CopyUncompressed;
		case CopyUncompressed:
			/* TODO: untested, slow, memcpy etc */
			/* s->nbits should be 0 here */
			while (s->lenpos) {
				if (s->src == s->srcend)
					return FLATE_IN;
				s->lenpos--;
				s->win[s->pos++] = *s->src++;
				if (s->pos == WinSize)
					return FLATE_OUT;
			}
			s->state = BlockHead;
			break;
		case FixedHuff:
			s->fixed = 1;
			s->state = DecodeBlock;
			break;
		case DynamicHuff:
			/* decode dynamic huffman code trees */
			if (!fillbits(s, 14))
				return FLATE_IN;
			s->nlit = 257 + getbits(s, 5);
			s->ndist = 1 + getbits(s, 5);
			s->nclen = 4 + getbits(s, 4);
			if (s->nlit > Nlitlen || s->ndist > Ndist)
				return s->err = "corrupt code tree.", FLATE_ERR;
			/* build code length tree */
			for (n = 0; n < Nclen; n++)
				s->lens[n] = 0;
			s->fixed = 0;
			s->state = DynamicHuffClen;
			s->lenpos = 0;
		case DynamicHuffClen:
			for (n = s->lenpos; n < s->nclen; n++)
				if (fillbits(s, 3)) {
					s->lens[clenorder[n]] = getbits(s, 3);
				} else {
					s->lenpos = n;
					return FLATE_IN;
				}
			/* using lhuff for code length huff code */
			if (build_huff(&s->lhuff, s->lens, Nclen, ClenTableBits) < 0)
				return s->err = "building clen tree failed.", FLATE_ERR;
			s->state = DynamicHuffLitlenDist;
			s->lenpos = 0;
		case DynamicHuffLitlenDist:
			/* decode code lengths for the dynamic trees */
			for (n = s->lenpos; n < s->nlit + s->ndist; ) {
				unsigned int len;
				unsigned char c;
				sym = decode_symbol(s, &s->lhuff);

				if (sym < 16) {
					s->lens[n++] = sym;
					continue;
				} else if (sym == (unsigned int)FLATE_IN) {
					s->lenpos = n;
					return FLATE_IN;
		case DynamicHuffContinue:
					n = s->lenpos;
					sym = s->nclen;
					s->state = DynamicHuffLitlenDist;
				}
				if (!fillbits(s, 7)) {
					/* TODO: 7 is too much when an almost empty block is at the end */
					if (sym == (unsigned int)FLATE_ERR)
						return FLATE_ERR;
					s->nclen = sym;
					s->lenpos = n;
					s->state = DynamicHuffContinue;
					return FLATE_IN;
				}
				/* TODO: bound check s->lens */
				if (sym == 16) {
					/* copy previous code length 3-6 times */
					c = s->lens[n - 1];
					for (len = 3 + getbits(s, 2); len; len--)
						s->lens[n++] = c;
				} else if (sym == 17) {
					/* repeat 0 for 3-10 times */
					for (len = 3 + getbits(s, 3); len; len--)
						s->lens[n++] = 0;
				} else if (sym == 18) {
					/* repeat 0 for 11-138 times */
					for (len = 11 + getbits(s, 7); len; len--)
						s->lens[n++] = 0;
				} else
					return s->err = "corrupt code tree.", FLATE_ERR;
			}
			/* build dynamic huffman code trees */
			if (build_huff(&s->lhuff, s->lens, s->nlit, LitlenTableBits) < 0)
				return s->err = "building litlen tree failed.", FLATE_ERR;
			if (build_huff(&s->dhuff, s->lens + s->nlit, s->ndist, DistTableBits) < 0)
				return s->err = "building dist tree failed.", FLATE_ERR;
			s->state = DecodeBlock;
		case DecodeBlock:
		case DecodeBlockLenBits:
		case DecodeBlockDist:
		case DecodeBlockDistBits:
		case DecodeBlockCopy:
			n = decode_block(s, s->fixed ? &lhuff : &s->lhuff, s->fixed ? &dhuff : &s->dhuff);
			if (n != FLATE_OK)
				return n;
			s->state = BlockHead;
			break;
		default:
			return s->err = "corrupt internal state.", FLATE_ERR;
		}
	}
}

static State *alloc_state(void) {
	State *s = (State*)malloc(sizeof(State));

	if (s) {
		s->final = s->pos = s->posout = s->bits = s->nbits = 0;
		s->state = BlockHead;
		s->src = s->srcend = 0;
		s->err = 0;
		/* TODO: globals.. */
		if (lhuff.nbits == 0)
			init_fixed_huffs();
	}
	return s;
}


/* extern */

int inflate(flat_stream *stream) {
	State *s = (State*)(stream->state);
	int n;

	if (stream->err) {
		if (s) {
			free(s);
			stream->state = 0;
		}
		return FLATE_ERR;
	}
	if (!s) {
		stream->state = alloc_state();
		s = (State*)(stream->state);
		if (!s)
			return stream->err = "no mem.", FLATE_ERR;
	}
	if (stream->avail_in) {
		s->src = stream->next_in;
		s->srcend = s->src + stream->avail_in;
		stream->avail_in = 0;
	}
	n = inflate_state(s);
	if (n == FLATE_OUT) {
		if (s->pos < stream->avail_out)
			stream->avail_out = s->pos;
		memcpy(stream->next_out, s->win + s->posout, stream->avail_out);
		s->pos -= stream->avail_out;
		if (s->pos)
			s->posout += stream->avail_out;
		else
			s->posout = 0;
	}
	if (n == FLATE_OK || n == FLATE_ERR) {
		if (s->nbits || s->src < s->srcend) {
			s->nbits /= 8;
			stream->next_in = s->src - s->nbits;
			stream->avail_in = s->srcend - s->src + s->nbits;
		}
		stream->err = s->err;
		free(s);
		stream->state = 0;
	}
	return n;
}

#ifdef __cplusplus
}
#endif
