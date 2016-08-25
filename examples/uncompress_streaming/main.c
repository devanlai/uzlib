/*
 * tgunzip -  gzip decompressor example
 *
 * Copyright (c) 2016 Currant, Inc.
 * All Rights Reserved
 * Modified to support a single byte output stream
 *  
 * 
 * Copyright (c) 2003 by Joergen Ibsen / Jibz
 * All Rights Reserved
 *
 * http://www.ibsensoftware.com/
 *
 * Copyright (c) 2014-2016 by Paul Sokolovsky
 *
 * This software is provided 'as-is', without any express
 * or implied warranty.  In no event will the authors be
 * held liable for any damages arising from the use of
 * this software.
 *
 * Permission is granted to anyone to use this software
 * for any purpose, including commercial applications,
 * and to alter it and redistribute it freely, subject to
 * the following restrictions:
 *
 * 1. The origin of this software must not be
 *    misrepresented; you must not claim that you
 *    wrote the original software. If you use this
 *    software in a product, an acknowledgment in
 *    the product documentation would be appreciated
 *    but is not required.
 *
 * 2. Altered source versions must be plainly marked
 *    as such, and must not be misrepresented as
 *    being the original software.
 *
 * 3. This notice may not be removed or altered from
 *    any source distribution.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <errno.h>

#include "tinf.h"

FILE *fin, *fout;

void exit_error(const char *what)
{
   printf("ERROR: %s\n", what);
   exit(1);
}

//readDest - read a byte from the decompressed destination file,
//           at 'offset' from the current position.
static unsigned char readDest(int offset)
{
  unsigned char ret;
  long last_pos = ftell(fout);

  int retval = fseek(fout, offset, SEEK_CUR);
  if (retval == -1) {
    printf("errno=%s\n", strerror(errno));
    exit_error("fseek pre");
  }   

  if (fread((unsigned char*)&ret, 1, 1, fout) != 1) exit_error("read");

  retval = fseek(fout, last_pos, SEEK_SET);
  if (retval == -1) {
    printf("errno=%s\n", strerror(errno));
    exit_error("fseek post");
  }   

  return ret;
}

static unsigned char readSource(struct TINF_DATA *data)
{
  unsigned char ret;
  if (fread((unsigned char*)&ret, 1, 1, fin) != 1) exit_error("read");
  return ret;
}

int main(int argc, char *argv[])
{
    unsigned int len, dlen, outlen;
    const unsigned char *source;
    int res;

    printf("tgunzip - example from the tiny inflate library (www.ibsensoftware.com)\n\n");

    if (argc < 3)
    {
       printf("Syntax: tgunzip <source> <destination>\n\n");
       return 1;
    }

    uzlib_init();

    /* -- open files -- */

    if ((fin = fopen(argv[1], "r")) == NULL) exit_error("source file");
    if ((fout = fopen(argv[2], "w+")) == NULL) exit_error("destination file");


    unsigned char dest;

    TINF_DATA d;
    outlen = 0;
    d.readSource = readSource;
    d.readDest = readDest;

    res = uzlib_gzip_parse_header(&d);
    if (res != TINF_OK) {
        printf("Error parsing header: %d\n", res);
        exit(1);
    }

    uzlib_uncompress_init(&d, NULL, 0);

    /* decompress a single byte at a time */

    do {
        d.destSize = 1;
        d.dest = &dest;
        res = uzlib_uncompress_chksum(&d);

        //if the destination has been written to, write it out to disk
        if (d.dest != &dest) {
            fwrite(&dest, 1, 1, fout);
            outlen++;
        }
        
        
    } while (res == TINF_OK);

    if (res != TINF_DONE) {
        printf("Error during decompression: %d\n", res);
    }

    printf("decompressed %d bytes\n", outlen);

    fclose(fin);
    fclose(fout);

    return 0;
}