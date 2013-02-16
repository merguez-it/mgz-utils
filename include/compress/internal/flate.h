#ifndef __MGZ_COMPRESS_INTERNALFLATE_H
#define __MGZ_COMPRESS_INTERNALFLATE_H

#include "compress/mgz_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

int mgz_deflate(mgz_stream *s);
int mgz_inflate(mgz_stream *s);

#ifdef __cplusplus
}
#endif

#endif // __MGZ_COMPRESS_INTERNALFLATE_H
