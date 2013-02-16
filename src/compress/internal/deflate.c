#include <stdlib.h>
#include <string.h>
#include "compress/internal/flate.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	CodeBits    = 16,      /* max number of bits in a code + 1 */
	EOB         = 256,     /* end of block symbol */
	Nlit        = 256,     /* number of lit codes */
	Nlen        = 29,      /* number of len codes */
	Nlitlen     = Nlit + Nlen + 3, /* litlen codes + block end + 2 unused */
	Ndist       = 30,      /* number of distance codes */
	Nclen       = 19,      /* number of code length codes */
	MinMatch    = 3,       /* min match length */
	MaxMatch    = 258,     /* max match length */
	WinSize     = 1 << 15, /* sliding window size */

	MaxChainLen = 256,     /* max length of hash chain */
	HashBits    = 13,
	HashSize    = 1 << HashBits, /* hash table size */
	BigDist     = 1 << 12, /* max match distance for short match length */
	MaxDist     = WinSize,
	BlockSize   = 1 << 15, /* TODO */
	SrcSize     = 2*WinSize + MaxMatch,
	DstSize     = BlockSize + MaxMatch + 6, /* worst case compressed block size */
	LzSize      = 1 << 13, /* lz buffer size */
	LzGuard     = LzSize - 2,
	LzLitFlag   = 1 << 15  /* marks literal run length in lz buffer */
};

typedef struct {
	unsigned short dist;
	unsigned short len;
} Match;

typedef struct {
	unsigned short n;
	unsigned short bits;
} LzCode;

typedef struct {
	int pos;               /* position in input src */
	int startpos;          /* block start pos in input src */
	int endpos;            /* end of available bytes in src */
	int skip;              /* skipped hash chain updates (until next iter) */
	Match prevm;           /* previous (deferred) match */
	int state;             /* prev return value */
	int eof;               /* end of input */
	unsigned char *in;             /* input data (not yet in src) */
	unsigned char *inend;
	unsigned int bits;             /* for output */
	int nbits;             /* for output */
	unsigned char *dst;            /* compressed output (position in dstbuf) */
	unsigned char *dstbegin;       /* start position of unflushed data in dstbuf */
	LzCode *lz;            /* current pos in lzbuf */
	int nlit;              /* literal run length in lzbuf */
	unsigned short head[HashSize]; /* position of hash chain heads */
	unsigned short chain[WinSize]; /* hash chain */
	unsigned short lfreq[Nlitlen];
	unsigned short dfreq[Ndist];
	unsigned char src[SrcSize];    /* input buf */
	unsigned char dstbuf[DstSize];
	LzCode lzbuf[LzSize];  /* literal run length, match len, match dist */
} State;

static unsigned char fixllen[Nlitlen]; /* fixed lit/len huffman code tree */
static unsigned short fixlcode[Nlitlen];
static unsigned char fixdlen[Ndist];   /* fixed distance huffman code tree */
static unsigned short fixdcode[Ndist];

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

static unsigned int revcode(unsigned int c, int n) {
	int i;
	unsigned int r = 0;

	for (i = 0; i < n; i++) {
		r = (r << 1) | (c & 1);
		c >>= 1;
	}
	return r;
}

/* build huffman code tree from code lengths */
static void huffcodes(unsigned short *codes, const unsigned char *lens, int n) {
	int c[CodeBits];
	int i, code, count;

	/* count code lengths and calc first code for each length */
	for (i = 0; i < CodeBits; i++)
		c[i] = 0;
	for (i = 0; i < n; i++)
		c[lens[i]]++;
	for (code = 0, i = 1; i < CodeBits; i++) {
		count = c[i];
		c[i] = code;
		code += count;
		if (code > (1 << i))
			abort(); /* over-subscribed */
		code <<= 1;
	}
	if (code < (1 << i)) {
		/* incomplete */;
		}

	for (i = 0; i < n; i++)
		if (lens[i])
			codes[i] = revcode(c[lens[i]]++, lens[i]);
		else
			codes[i] = 0;
}

static int heapparent(int n) {return (n - 2)/4 * 2;}
static int heapchild(int n)  {return 2 * n + 2;}

static int heappush(int *heap, int len, int w, int n) {
	int p, c, tmp;

	c = len;
	heap[len++] = n;
	heap[len++] = w;
	while (c > 0) {
		p = heapparent(c);
		if (heap[c+1] < heap[p+1]) {
			tmp = heap[c]; heap[c] = heap[p]; heap[p] = tmp;
			tmp = heap[c+1]; heap[c+1] = heap[p+1]; heap[p+1] = tmp;
			c = p;
		} else
			break;
	}
	return len;
}

static int heappop(int *heap, int len, int *w, int *n) {
	int p, c, tmp;

	*n = heap[0];
	*w = heap[1];
	heap[1] = heap[--len];
	heap[0] = heap[--len];
	p = 0;
	for (;;) {
		c = heapchild(p);
		if (c >= len)
			break;
		if (c+2 < len && heap[c+3] < heap[c+1])
			c += 2;
		if (heap[p+1] > heap[c+1]) {
			tmp = heap[p]; heap[p] = heap[c]; heap[c] = tmp;
			tmp = heap[p+1]; heap[p+1] = heap[c+1]; heap[c+1] = tmp;
		} else
			break;
		p = c;
	}
	return len;
}

/* symbol frequencies -> code lengths (limited to 255) */
static void hufflens(unsigned char *lens, unsigned short *freqs, int nsym, int limit) {
	/* 2 <= nsym <= Nlitlen, log(nsym) <= limit <= CodeBits-1 */
	int parent[2*Nlitlen-1];
	int count[CodeBits];
	int heap[2*Nlitlen];
	int n, len, top, overflow;
	int i, j;
	int wi, wj;

	for (n = 0; n < limit+1; n++)
		count[n] = 0;
	for (len = n = 0; n < nsym; n++)
		if (freqs[n] > 0)
			len = heappush(heap, len, freqs[n], n);
		else
			lens[n] = 0;
	/* deflate: fewer than two symbols: add new */
	for (n = 0; len < 4; n++)
		if (freqs[n] == 0)
			len = heappush(heap, len, ++freqs[n], n);
	/* build code tree */
	top = len;
	for (n = nsym; len > 2; n++) {
		len = heappop(heap, len, &wi, &i);
		len = heappop(heap, len, &wj, &j);
		parent[i] = n;
		parent[j] = n;
		len = heappush(heap, len, wi + wj, n);
		/* keep an ordered list of nodes at the end */
		heap[len+1] = i;
		heap[len] = j;
	}
	/* calc code lengths (deflate: with limit) */
	overflow = 0;
	parent[--n] = 0;
	for (i = 2; i < top; i++) {
		n = heap[i];
		if (n >= nsym) {
			/* overwrite parent index with length */
			parent[n] = parent[parent[n]] + 1;
			if (parent[n] > limit)
				overflow++;
		} else {
			lens[n] = parent[parent[n]] + 1;
			if (lens[n] > limit) {
				lens[n] = limit;
				overflow++;
			}
			count[lens[n]]++;
		}
	}
	if (overflow == 0)
		return;
	/* modify code tree to fix overflow (from zlib) */
	while (overflow > 0) {
		for (n = limit-1; count[n] == 0; n--);
		count[n]--;
		count[n+1] += 2;
		count[limit]--;
		overflow -= 2;
	}
	for (len = limit; len > 0; len--)
		for (i = count[len]; i > 0;) {
			n = heap[--top];
			if (n < nsym) {
				lens[n] = len;
				i--;
			}
		}
}

/* output n (<= 16) bits */
static void putbits(State *s, unsigned int bits, int n) {
	s->bits |= bits << s->nbits;
	s->nbits += n;
	while (s->nbits >= 8) {
		*s->dst++ = s->bits & 0xff;
		s->bits >>= 8;
		s->nbits -= 8;
	}
}

/* run length encode literal and dist code lengths into codes and extra */
static int clencodes(unsigned char *codes, unsigned char *extra, unsigned char *llen, int nlit, unsigned char *dlen, int ndist) {
	int i, c, r, rr;
	int n = 0;

	for (i = 0; i < nlit; i++)
		codes[i] = llen[i];
	for (i = 0; i < ndist; i++)
		codes[nlit + i] = dlen[i];
	for (i = 0; i < nlit + ndist;) {
		c = codes[i];
		for (r = 1; i + r < nlit + ndist && codes[i + r] == c; r++);
		i += r;
		if (c == 0) {
			while (r >= 11) {
				rr = r > 138 ? 138 : r;
				codes[n] = 18;
				extra[n++] = rr - 11;
				r -= rr;
			}
			if (r >= 3) {
				codes[n] = 17;
				extra[n++] = r - 3;
				r = 0;
			}
		}
		while (r--) {
			codes[n++] = c;
			while (r >= 3) {
				rr = r > 6 ? 6 : r;
				codes[n] = 16;
				extra[n++] = rr - 3;
				r -= rr;
			}
		}
	}
	return n;
}

/* compress block data into s->dstbuf using given codes */
static void putblock(State *s, unsigned short *lcode, unsigned char *llen, unsigned short *dcode, unsigned char *dlen) {
	int n;
	LzCode *lz;
	unsigned char *p;

	for (lz = s->lzbuf, p = s->src + s->startpos; lz != s->lz; lz++)
		if (lz->bits & LzLitFlag)
			for (n = lz->n; n > 0; n--, p++)
				putbits(s, lcode[*p], llen[*p]);
		else {
			p += lenbase[lz->n] + lz->bits;
			putbits(s, lcode[Nlit + lz->n + 1], llen[Nlit + lz->n + 1]);
			putbits(s, lz->bits, lenbits[lz->n]);
			lz++;
			putbits(s, dcode[lz->n], dlen[lz->n]);
			putbits(s, lz->bits, distbits[lz->n]);
		}
	putbits(s, lcode[EOB], llen[EOB]);
}

/* build code trees and select dynamic/fixed/uncompressed block compression */
static void deflate_block(State *s) {
	unsigned char codes[Nlitlen + Ndist], extra[Nlitlen + Ndist];
	unsigned char llen[Nlitlen], dlen[Ndist], clen[Nclen];
	unsigned short cfreq[Nclen];
	/* freq can be overwritten by code */
	unsigned short *lcode = s->lfreq, *dcode = s->dfreq, *ccode = cfreq;
	int i, c, n, ncodes;
	int nlit, ndist, nclen;
	LzCode *lz;
	unsigned char *p;
	int dynsize, fixsize, uncsize;
	int blocklen = s->pos - s->startpos;
/* int dyntree; */

	/* calc dynamic codes */
	hufflens(llen, s->lfreq, Nlitlen, CodeBits-1);
	hufflens(dlen, s->dfreq, Ndist, CodeBits-1);
	huffcodes(lcode, llen, Nlitlen);
	huffcodes(dcode, dlen, Ndist);
	for (nlit = Nlitlen; nlit > Nlit && llen[nlit-1] == 0; nlit--);
	for (ndist = Ndist; ndist > 1 && dlen[ndist-1] == 0; ndist--);
	ncodes = clencodes(codes, extra, llen, nlit, dlen, ndist);
	memset(cfreq, 0, sizeof(cfreq));
	for (i = 0; i < ncodes; i++)
		cfreq[codes[i]]++;
	hufflens(clen, cfreq, Nclen, 7);
	huffcodes(ccode, clen, Nclen);
	for (nclen = Nclen; nclen > 4 && clen[clenorder[nclen-1]] == 0; nclen--);

	/* calc compressed size */
	uncsize = 3 + 16 + 8 * blocklen + (16 - 3 - s->nbits) % 8; /* byte aligned */
	fixsize = 3;
	dynsize = 3 + 5 + 5 + 4 + 3 * nclen;
	for (i = 0; i < ncodes; i++) {
		c = codes[i];
		dynsize += clen[c];
		if (c == 16)
			dynsize += 2;
		if (c == 17)
			dynsize += 3;
		if (c == 18)
			dynsize += 7;
	}
/* dyntree = dynsize - 3; */
	for (lz = s->lzbuf, p = s->src + s->startpos; lz != s->lz; lz++)
		if (lz->bits & LzLitFlag)
			for (n = lz->n; n > 0; n--, p++) {
				fixsize += fixllen[*p];
				dynsize += llen[*p];
			}
		else {
			p += lenbase[lz->n] + lz->bits;
			fixsize += fixllen[Nlit + lz->n + 1];
			dynsize += llen[Nlit + lz->n + 1];
			fixsize += lenbits[lz->n];
			dynsize += lenbits[lz->n];
			lz++;
			fixsize += fixdlen[lz->n];
			dynsize += dlen[lz->n];
			fixsize += distbits[lz->n];
			dynsize += distbits[lz->n];
		}
	fixsize += fixllen[EOB];
	dynsize += llen[EOB];

	/* emit block */
	putbits(s, s->eof && s->pos == s->endpos, 1);
	if (dynsize < fixsize && dynsize < uncsize) {
		/* dynamic code */
		putbits(s, 2, 2);
		putbits(s, nlit - 257, 5);
		putbits(s, ndist - 1, 5);
		putbits(s, nclen - 4, 4);
		for (i = 0; i < nclen; i++)
			putbits(s, clen[clenorder[i]], 3);
		for (i = 0; i < ncodes; i++) {
			c = codes[i];
			putbits(s, ccode[c], clen[c]);
			if (c == 16)
				putbits(s, extra[i], 2);
			if (c == 17)
				putbits(s, extra[i], 3);
			if (c == 18)
				putbits(s, extra[i], 7);
		}
		putblock(s, lcode, llen, dcode, dlen);
	} else if (fixsize < uncsize) {
		/* fixed code */
		putbits(s, 1, 2);
		putblock(s, fixlcode, fixllen, fixdcode, fixdlen);
	} else {
		/* uncompressed */
		putbits(s, 0, 2);
		putbits(s, 0, 7); /* align to byte boundary */
		s->nbits = 0;
		putbits(s, blocklen, 16);
		putbits(s, ~blocklen & 0xffff, 16);
		memcpy(s->dst, s->src + s->startpos, blocklen);
		s->dst += blocklen;
	}
/*
fprintf(stderr, "blen:%d [%d,%d] lzlen:%d dynlen:%d (tree:%d rate:%.3f) fixlen:%d (rate:%.3f) unclen:%d (rate:%.3f)\n",
	blocklen, s->startpos, s->pos, s->lz - s->lzbuf, dynsize, dyntree, dynsize/(float)blocklen,
	fixsize, fixsize/(float)blocklen, uncsize, uncsize/(float)blocklen);
*/
}

/* find n in base */
static int bisect(unsigned short *base, int len, int n) {
	int lo = 0;
	int hi = len;
	int k;

	while (lo < hi) {
		k = (lo + hi) / 2;
		if (n < base[k])
			hi = k;
		else
			lo = k + 1;
	}
	return lo - 1;
}

/* add literal run length to lzbuf */
static void flushlit(State *s) {
	if (s->nlit) {
		s->lz->bits = LzLitFlag;
		s->lz->n = s->nlit;
		s->lz++;
		s->nlit = 0;
	}
}

/* add match to lzbuf and update freq counts */
static void recordmatch(State *s, Match m) {
	int n;

/*fprintf(stderr, "m %d %d\n", m.len, m.dist);*/
	flushlit(s);
	n = bisect(lenbase, Nlen, m.len);
	s->lz->n = n;
	s->lz->bits = m.len - lenbase[n];
	s->lz++;
	s->lfreq[Nlit + n + 1]++;
	n = bisect(distbase, Ndist, m.dist);
	s->lz->n = n;
	s->lz->bits = m.dist - distbase[n];
	s->lz++;
	s->dfreq[n]++;
}

/* update literal run length */
static void recordlit(State *s, int c) {
/*fprintf(stderr, "l %c\n", c);*/
	s->nlit++;
	s->lfreq[c]++;
}

/* multiplicative hash (using a prime close to golden ratio * 2^32) */
static int gethash(unsigned char *p) {
	return (0x9e3779b1 * ((p[0]<<16) + (p[1]<<8) + p[2]) >> (32 - HashBits)) % HashSize;
}

/* update hash chain at the current position */
static int updatechain(State *s) {
	int hash, next = 0, p = s->pos, i;

	if (s->endpos - p < MinMatch)
		p = s->endpos - MinMatch;
	for (i = s->pos - s->skip; i <= p; i++) {
		hash = gethash(s->src + i);
		next = s->head[hash];
		s->head[hash] = i;
		if (next >= i || i - next >= MaxDist)
			next = 0;
		s->chain[i % WinSize] = next;
	}
	s->skip = 0;
	return next;
}

/* find longest match, next position in the hash chain is given */
static Match getmatch(State *s, int next) {
	Match m = {0, MinMatch-1};
	int len;
	int limit = s->pos - MaxDist;
	int chainlen = MaxChainLen;
	unsigned char *q;
	unsigned char *p = s->src + s->pos;
	unsigned char *end = p + MaxMatch;

	do {
		q = s->src + next;
/*fprintf(stderr,"match: next:%d pos:%d limit:%d\n", next, s->pos, limit);*/
		/* next match should be at least m.len+1 long */
		if (q[m.len] != p[m.len] || q[m.len-1] != p[m.len-1] || q[0] != p[0])
			continue;
		while (++p != end && *++q == *p);
		len = MaxMatch - (end - p);
		p -= len;
/*fprintf(stderr,"match: len:%d dist:%d\n", len, s->pos - next);*/
		if (len > m.len) {
			m.dist = s->pos - next;
			m.len = len;
			if (s->pos + len >= s->endpos) { /* TODO: overflow */
				m.len = s->endpos - s->pos;
				return m;
			}
			if (len == MaxMatch)
				return m;
		}
	} while ((next = s->chain[next % WinSize]) > limit && --chainlen);
	if (m.len < MinMatch || (m.len == MinMatch && m.dist > BigDist))
		m.len = 0;
	return m;
}

static void startblock(State *s) {
	s->startpos = s->pos;
	s->dst = s->dstbegin = s->dstbuf;
	s->lz = s->lzbuf;
	s->nlit = 0;
	memset(s->lfreq, 0, sizeof(s->lfreq));
	memset(s->dfreq, 0, sizeof(s->dfreq));
	s->lfreq[EOB]++;
}

static int shiftwin(State *s) {
	int n;

	if (s->startpos < WinSize)
		return 0;
	memmove(s->src, s->src + WinSize, SrcSize - WinSize);
	for (n = 0; n < HashSize; n++)
		s->head[n] = s->head[n] > WinSize ? s->head[n] - WinSize : 0;
	for (n = 0; n < WinSize; n++)
		s->chain[n] = s->chain[n] > WinSize ? s->chain[n] - WinSize : 0;
	s->pos -= WinSize;
	s->startpos -= WinSize;
	s->endpos -= WinSize;
	return 1;
}

static int endblock(State *s) {
	if ((s->pos >= 2*WinSize && !shiftwin(s)) || s->pos - s->startpos >= BlockSize ||
	  s->lz - s->lzbuf >= LzGuard || (s->eof && s->pos == s->endpos)) {
		/* deflate block */
		flushlit(s);
		if (s->prevm.len)
			s->pos--;
		deflate_block(s);
		if (s->eof && s->pos == s->endpos)
			putbits(s, 0, 7);
		return 1;
	}
	return 0;
}

static int fillsrc(State *s) {
	int n, k;

	if (s->endpos < SrcSize && !s->eof) {
		n = SrcSize - s->endpos;
		k = s->inend - s->in;
		if (n > k)
			n = k;
		memcpy(s->src + s->endpos, s->in, n);
		s->in += n;
		s->endpos += n;
		if (s->endpos < SrcSize)
			return 0;
	}
	return 1;
}

static int calcguard(State *s) {
	int p = s->endpos - MaxMatch;
	int q = s->startpos + BlockSize;

	return p < q ? p : q;
}

/* deflate compress from s->src into s->dstbuf */
static int deflate_state(State *s) {
	Match m;
	int next;
	int guard;

	if (s->state == FLATE_IN)
		s->eof = s->in == s->inend;
	else if (s->state == FLATE_OUT) {
		if (s->dstbegin < s->dst)
			return (s->state = FLATE_OUT);
		if (s->eof && s->pos == s->endpos)
			return (s->state = FLATE_OK);
		startblock(s);
		if (s->prevm.len)
			s->pos++;
	} else
		return s->state;

	guard = calcguard(s);
	for (;;) {
		if (s->pos >= guard || s->lz - s->lzbuf >= LzGuard) {
/*fprintf(stderr,"guard:%d pos:%d len:%d lzlen:%d end:%d start:%d nin:%d eof:%d\n", guard, s->pos, s->pos - s->startpos, s->lz - s->lzbuf, s->endpos, s->startpos, s->inend - s->in, s->eof);*/
			if (endblock(s))
				return (s->state = FLATE_OUT);
			if (!fillsrc(s))
				return (s->state = FLATE_IN);
			guard = calcguard(s);
		}
		next = updatechain(s);
		if (next)
			m = getmatch(s, next);
		if (next && m.len > s->prevm.len) {
			if (s->prevm.len)
				recordlit(s, s->src[s->pos-1]);
			s->prevm = m;
		} else if (s->prevm.len) {
			recordmatch(s, s->prevm);
			s->skip = s->prevm.len - 2;
			s->prevm.len = 0;
			s->pos += s->skip;
		} else
			recordlit(s, s->src[s->pos]);
		s->pos++;
	}
}

/* alloc and init state */
static State *alloc_state(void) {
	State *s = (State*)malloc(sizeof(State));
	int i;

	if (!s)
		return s;
	memset(s->chain, 0, sizeof(s->chain));
	memset(s->head, 0, sizeof(s->head));
	s->bits = s->nbits = 0;
	/* TODO: globals */
	if (fixllen[0] == 0) {
		for (i = 0; i < 144; i++)
			fixllen[i] = 8;
		for (; i < 256; i++)
			fixllen[i] = 9;
		for (; i < 280; i++)
			fixllen[i] = 7;
		for (; i < Nlitlen; i++)
			fixllen[i] = 8;
		for (i = 0; i < Ndist; i++)
			fixdlen[i] = 5;
		huffcodes(fixlcode, fixllen, Nlitlen);
		huffcodes(fixdcode, fixdlen, Ndist);
	}
	s->state = FLATE_OUT;
	s->in = s->inend = 0;
	s->dst = s->dstbegin = s->dstbuf;
	s->pos = s->startpos = s->endpos = WinSize;
	s->eof = 0;
	s->skip = 0;
	s->prevm.len = 0;
	return s;
}


/* extern */

int mgz_deflate(mgz_stream *stream) {
	State *s = (State*)(stream->state);
	int n, k;

	if (stream->err) {
		free(s);
		stream->state = 0;
		return FLATE_ERR;
	}
	if (!s) {
		stream->state = alloc_state();
		s = (State*)(stream->state);
		if (!s)
			return stream->err = "no mem.", FLATE_ERR;
	}
	if (stream->avail_in) {
		s->in = stream->next_in;
		s->inend = s->in + stream->avail_in;
		stream->avail_in = 0;
	}
	n = deflate_state(s);
	if (n == FLATE_OUT) {
		k = s->dst - s->dstbegin;
		if (k < stream->avail_out)
			stream->avail_out = k;
		memcpy(stream->next_out, s->dstbegin, stream->avail_out);
		s->dstbegin += stream->avail_out;
	}
	if (n == FLATE_OK || n == FLATE_ERR) {
		free(s);
		stream->state = 0;
	}
	return n;
}

#ifdef __cplusplus
}
#endif
