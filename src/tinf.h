/*
 * uzlib  -  tiny deflate/inflate library (deflate, gzip, zlib)
 *
 * Copyright (c) 2003 by Joergen Ibsen / Jibz
 * All Rights Reserved
 * http://www.ibsensoftware.com/
 *
 * Copyright (c) 2014-2016 by Paul Sokolovsky
 */

#ifndef TINF_H_INCLUDED
#define TINF_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

/* calling convention */
#ifndef TINFCC
 #ifdef __WATCOMC__
  #define TINFCC __cdecl
 #else
  #define TINFCC
 #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    /* ok status, more data produced */
    TINF_OK = 0,
    /* end of compressed stream reached */
    TINF_DONE = 1,
    TINF_DATA_ERROR = -3,
    TINF_CHKSUM_ERROR = -4
} TINF_STATUS;

/* checksum types */
typedef enum {
    TINF_CHKSUM_NONE = 0,
    TINF_CHKSUM_ADLER = 1,
    TINF_CHKSUM_CRC = 2,
} TINF_CHKSUM_TYPE;

/* data structures */

typedef struct {
   uint16_t table[16];  /* table of code length counts */
   uint16_t trans[288]; /* code -> symbol translation table */
} TINF_TREE;

struct TINF_DATA;
typedef struct TINF_DATA {
    const uint8_t *source;
    /* If source above is NULL, this function will be used to read
       next byte from source stream */
    TINF_STATUS (*readSourceByte)(uint8_t *out);

    unsigned int tag;
    unsigned int bitcount;

    /* Current pointer in the output buffer */
    uint8_t *dest;

    /* if readDestByte is provided, it will use this function for
       reading from the output stream, rather than assuming
       'dest' contains the entire output stream in memory
    */
    TINF_STATUS (*readDestByte)(int offset, uint8_t *out);

    /* if writeDestByte is provided, it will use this function for
       writing to the output stream, rather than writing into
       the 'dest' buffer.
    */
    void (*writeDestByte)(uint8_t datum);

    /* Accumulating checksum */
    uint32_t checksum;
    TINF_CHKSUM_TYPE checksum_type;

    int btype;
    int bfinal;
    unsigned int curlen;
    int lzOff;
    uint8_t *dict_ring;
    size_t dict_size;
    size_t dict_idx;

    TINF_TREE ltree; /* dynamic length/symbol tree */
    TINF_TREE dtree; /* dynamic distance tree */
} TINF_DATA;

#define TINF_PUT(d, c) \
    { \
        if (d->writeDestByte) { \
            d->writeDestByte(c); \
        } else { \
            *d->dest++ = c; \
        } \
        if (d->dict_ring) { \
            d->dict_ring[d->dict_idx++] = c; \
            if (d->dict_idx == d->dict_size) { \
                d->dict_idx = 0; \
            } \
        } \
    }

uint8_t TINFCC uzlib_get_byte(TINF_DATA *d);

/* Decompression API */

void TINFCC uzlib_init(void);
void TINFCC uzlib_uncompress_init(TINF_DATA *d, void *dict, size_t dictLen);
TINF_STATUS TINFCC uzlib_uncompress(TINF_DATA *d, size_t out_len);
TINF_STATUS TINFCC uzlib_uncompress_chksum(TINF_DATA *d, size_t out_len);

uint8_t TINFCC uzlib_zlib_parse_header(TINF_DATA *d);
TINF_STATUS TINFCC uzlib_gzip_parse_header(TINF_DATA *d);

/* Compression API */

void TINFCC uzlib_compress(void *data, const uint8_t *src, size_t slen);

/* Checksum API */

/* prev_sum is previous value for incremental computation, 1 initially */
uint32_t TINFCC uzlib_adler32(const void *data, size_t length, uint32_t prev_sum);
/* crc is previous value for incremental computation, 0xffffffff initially */
uint32_t TINFCC uzlib_crc32(const void *data, size_t length, uint32_t crc);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TINF_H_INCLUDED */
