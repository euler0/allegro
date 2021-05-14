/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *      By Shawn Hargreaves,
 *      1 Salisbury Road,
 *      Market Drayton,
 *      Shropshire,
 *      England, TF9 1AJ.
 *
 *      File I/O and LZSS compression routines.
 *
 *      See readme.txt for copyright information.
 */

#include <errno.h>
#include <dos.h>
#include <io.h>
#include <dir.h>
#include <stdlib.h>
#include <unistd.h>

#include "allegro.h"


#define N            4096     /* 4k buffers for LZ compression */
#define F            18       /* upper limit for LZ match length */
#define THRESHOLD    2        /* LZ encode string into position and length
				 if match length is greater than this */


typedef struct PACK_DATA            /* stuff for doing LZ compression */
{
   PACKFILE *f;                     /* the 'real' file */
   int state;                       /* where have we got to in the pack? */
   int i, c, len, r, s;
   int last_match_length, code_buf_ptr;
   unsigned char mask;
   char code_buf[17];
   int match_position;
   int match_length;
   int lson[N+1];                   /* left children, */
   int rson[N+257];                 /* right children, */
   int dad[N+1];                    /* and parents, = binary search trees */
   unsigned char text_buf[N+F-1];   /* ring buffer, with F-1 extra bytes
				       for string comparison */
} PACK_DATA;


typedef struct UNPACK_DATA          /* for reading LZ files */
{
   PACKFILE *f;                     /* the 'real' file */
   int state;                       /* where have we got to? */
   int i, j, k, r, c;
   int flags;
   unsigned char text_buf[N+F-1];   /* ring buffer, with F-1 extra bytes
				       for string comparison */
} UNPACK_DATA;


static int refill_buffer(PACKFILE *f);
static int flush_buffer(PACKFILE *f, int last);
static void pack_inittree(PACK_DATA *dat);
static void pack_insertnode(int r, PACK_DATA *dat);
static void pack_deletenode(int p, PACK_DATA *dat);
static int pack_write(PACK_DATA *dat, int size, unsigned char *buf, int last);
static int pack_read(UNPACK_DATA *dat, int s, unsigned char *buf);



/* get_filename:
 *  When passed a completely specified file path, this returns a pointer
 *  to the filename portion. Both '\' and '/' are recognized as directory
 *  separators.
 */
char *get_filename(char *path)
{
   int pos;

   for (pos=0; path[pos]; pos++)
      ; /* do nothing */ 

   while ((pos>0) && (path[pos-1] != '\\') && (path[pos-1] != '/'))
      pos--;

   return path+pos;
}



/* get_extension:
 *  When passed a complete filename (with or without path information)
 *  this returns a pointer to the file extension.
 */
char *get_extension(char *filename)
{
   int pos, end;

   for (end=0; filename[end]; end++)
      ; /* do nothing */

   pos = end;

   while ((pos>0) && (filename[pos-1] != '.') &&
	  (filename[pos-1] != '\\') && (filename[pos-1] != '/'))
      pos--;

   if (filename[pos-1] == '.')
      return filename+pos;

   return filename+end;
}



/* put_backslash:
 *  If the last character of the filename is not a '\' or '/', this routine
 *  will concatenate a '\' on to it.
 */
void put_backslash(char *filename)
{
   int i = strlen(filename);

   if (i>0)
      if ((filename[i-1]=='\\') || (filename[i-1]=='/'))
	 return;

   filename[i++] = '\\';
   filename[i] = 0;
}



/* file_exists:
 *  Checks whether a file matching the given name and attributes exists,
 *  returning non zero if it does. The file attribute may contain any of
 *  the FA_* constants from dir.h. If aret is not null, it will be set 
 *  to the attributes of the matching file. If an error occurs the system 
 *  error code will be stored in errno.
 */
int file_exists(char *filename, int attrib, int *aret)
{
   struct find_t dta;

   errno = _dos_findfirst(filename, attrib, &dta);

   if (aret)
      *aret = dta.attrib;

   return ((errno) ? 0 : -1);
}



/* file_size:
 *  Returns the size of a file, in bytes.
 *  If the file does not exist or an error occurs, it will return zero
 *  and store the system error code in errno.
 */
long file_size(char *filename)
{
   struct find_t dta;

   errno = _dos_findfirst(filename, FA_RDONLY | FA_HIDDEN | FA_ARCH, &dta);

   if (errno)
      return 0L;
   else
      return dta.size;
}



/* delete_file:
 *  Removes a file from the disk.
 */
int delete_file(char *filename)
{
   unlink(filename);
   return errno;
}



/* for_each_file:
 *  Finds all the files on the disk which match the given wildcard
 *  specification and file attributes, and executes callback() once for
 *  each. callback() will be passed three arguments, the first a string
 *  which contains the completed filename, the second being the attributes
 *  of the file, and the third an int which is simply a copy of param (you
 *  can use this for whatever you like). If an error occurs an error code
 *  will be stored in errno, and callback() can cause for_each_file() to
 *  abort by setting errno itself. Returns the number of successful calls
 *  made to callback(). The file attribute may contain any of the FA_* 
 *  flags from dir.h.
 */
int for_each_file(char *name, int attrib, void (*callback)(), int param)
{
   char buf[80];
   int c = 0;
   struct find_t dta;

   errno = _dos_findfirst(name, attrib, &dta);
   if (errno!=0) {
      return 0;
   }

   do {
      strcpy(buf,name);
      strcpy(get_filename(buf),dta.name);
      (*callback)(buf,dta.attrib,param);
      if (errno!=0)
	 break;
      c++;
   } while ((errno=_dos_findnext(&dta))==0);

   errno = 0;
   return c;
}



/* pack_fopen:
 *  Opens a file according to mode, which may contain any of the flags:
 *  'r': open file for reading.
 *  'w': open file for writing, overwriting any existing data.
 *  'p': open file in 'packed' mode. Data will be compressed as it is
 *       written to the file, and automatically uncompressed during read
 *       operations. Files created in this mode will produce garbage if
 *       they are read without this flag being set.
 *  '!': open file for writing in normal, unpacked mode, but add the value
 *       F_NOPACK_MAGIC to the start of the file, so that it can be opened
 *       in packed mode and Allegro will automatically detect that the
 *       data does not need to be decompressed.
 *
 *  Instead of these flags, one of the constants F_READ, F_WRITE,
 *  F_READ_PACKED, F_WRITE_PACKED or F_WRITE_NOPACK may be used as the second 
 *  argument to fopen().
 *
 *  On success, fopen() returns a pointer to a file structure, and on error
 *  it returns NULL and stores an error code in errno. An attempt to read a 
 *  normal file in packed mode will cause errno to be set to EDOM.
 */
PACKFILE *pack_fopen(char *filename, char *mode)
{
   PACKFILE *f, *f2;
   struct find_t dta;
   int c;
   long header = FALSE;

   errno=0;

   if ((f=malloc(sizeof(PACKFILE)))==(PACKFILE *)NULL) {
      errno=ENOMEM;
      return NULL;
   }

   f->buf_pos = f->buf;
   f->write = f->pack = f->eof = f->error = FALSE;
   f->buf_size = 0;

   for (c=0; mode[c]; c++) {
      switch(mode[c]) {
	 case 'r': case 'R': f->write = FALSE; break;
	 case 'w': case 'W': f->write = TRUE; break;
	 case 'p': case 'P': f->pack = TRUE; break;
	 case '!': f->pack = FALSE; header = TRUE; break;
      }
   }

   if (f->write) {
      if (f->pack) {
	 /* write a packed file */
	 PACK_DATA *dat = malloc(sizeof(PACK_DATA));
	 if (!dat) {
	    errno = ENOMEM;
	    free(f);
	    return NULL;
	 }
	 if ((dat->f = pack_fopen(filename,F_WRITE)) == (PACKFILE *)NULL) {
	    free(dat);
	    free(f);
	    return NULL;
	 }
	 pack_mputl(F_PACK_MAGIC,dat->f); /* can't fail (buffer not full) */
	 for (c=0; c < N - F; c++)
	    dat->text_buf[c] = 0; 
	 dat->state = 0;
	 f->pack_data = (char *)dat;
      }
      else {
	 /* write a 'real' file */
	 f->pack_data = NULL;
	 if (_dos_creat(filename, _A_NORMAL, &f->hndl) != 0) {
	    free(f);
	    return NULL;
	 }
	 errno = 0;
      }
      f->todo = 0L;
      if (header)
	 pack_mputl(F_NOPACK_MAGIC,f);    /* can't fail (buffer not full) */
   }

   else {                        /* must be a read */
      if (f->pack) {
	 /* read a packed file */
	 UNPACK_DATA *dat = malloc(sizeof(UNPACK_DATA));
	 if (!dat) {
	    errno = ENOMEM;
	    free(f);
	    return NULL;
	 }
	 if ((dat->f = pack_fopen(filename,F_READ)) == (PACKFILE *)NULL) {
	    free(dat);
	    free(f);
	    return NULL;
	 }
	 header = pack_mgetl(dat->f);
	 if (header == F_PACK_MAGIC) {
	    for (c=0; c < N - F; c++)
	       dat->text_buf[c] = 0; 
	    dat->state = 0;
	    f->todo = 0xffff;
	    f->pack_data = (char *)dat;
	 }
	 else {
	    if (header == F_NOPACK_MAGIC) {
	       f2 = dat->f;
	       free(dat);
	       free(f);
	       return f2;
	    }
	    else {
	       pack_fclose(dat->f);
	       free(dat);
	       free(f);
	       if (errno==0)
		  errno = EDOM;
	       return NULL;
	    }
	 }
      }
      else {
	 /* read a 'real' file */
	 f->pack_data = NULL;
	 errno = _dos_findfirst(filename, FA_RDONLY | FA_HIDDEN | FA_ARCH, &dta);
	 if (errno != 0) {
	    free(f);
	    return NULL;
	 }
	 f->todo = dta.size;
	 if (_dos_open(filename, 0, &f->hndl) != 0) {
	    errno = f->hndl;
	    free(f);
	    return NULL;
	 }
      }
   }
   return f;
}



/* pack_fclose:
 *  Closes a file after it has been read or written.
 *  Returns zero on success. On error it returns an error code which is
 *  also stored in errno. This function can fail only when writing to
 *  files: if the file was opened in read mode it will always succeed.
 */
int pack_fclose(PACKFILE *f)
{
   if (f) {
      if (f->write)
	 flush_buffer(f,TRUE);

      if (f->pack_data) {
	 if (f->write) {
	    PACK_DATA *dat = (PACK_DATA *)f->pack_data;
	    pack_fclose(dat->f);
	 }
	 else {
	    UNPACK_DATA *dat = (UNPACK_DATA *)f->pack_data;
	    pack_fclose(dat->f);
	 }
	 free(f->pack_data);
      }
      else
	 _dos_close(f->hndl);

      free(f);
      return errno;
   }

   return 0;
}



/* pack_igetw:
 *  Reads a 16 bit word from a file, using intel byte ordering.
 */
int pack_igetw(PACKFILE *f)
{
   int b1, b2;

   if ((b1 = pack_getc(f)) != EOF)
      if ((b2 = pack_getc(f)) != EOF)
	 return ((b2 << 8) | b1);

   return EOF;
}



/* pack_igetl:
 *  Reads a 32 bit long from a file, using intel byte ordering.
 */
long pack_igetl(PACKFILE *f)
{
   int b1, b2, b3, b4;

   if ((b1 = pack_getc(f)) != EOF)
      if ((b2 = pack_getc(f)) != EOF)
	 if ((b3 = pack_getc(f)) != EOF)
	    if ((b4 = pack_getc(f)) != EOF)
	       return (((long)b4 << 24) | ((long)b3 << 16) |
		       ((long)b2 << 8) | (long)b1);

   return EOF;
}



/* pack_iputw:
 *  Writes a 16 bit int to a file, using intel byte ordering.
 */
int pack_iputw(int w, PACKFILE *f)
{
   int b1, b2;

   b1 = (w & 0xff00) >> 8;
   b2 = w & 0x00ff;

   if (pack_putc(b2,f)==b2)
      if (pack_putc(b1,f)==b1)
	 return w;

   return EOF;
}



/* pack_iputw:
 *  Writes a 32 bit long to a file, using intel byte ordering.
 */
long pack_iputl(long l, PACKFILE *f)
{
   int b1, b2, b3, b4;

   b1 = (int)((l & 0xff000000L) >> 24);
   b2 = (int)((l & 0x00ff0000L) >> 16);
   b3 = (int)((l & 0x0000ff00L) >> 8);
   b4 = (int)l & 0x00ff;

   if (pack_putc(b4,f)==b4)
      if (pack_putc(b3,f)==b3)
	 if (pack_putc(b2,f)==b2)
	    if (pack_putc(b1,f)==b1)
	       return l;

   return EOF;
}



/* pack_mgetw:
 *  Reads a 16 bit int from a file, using motorola byte-ordering.
 */
int pack_mgetw(PACKFILE *f)
{
   int b1, b2;

   if ((b1 = pack_getc(f)) != EOF)
      if ((b2 = pack_getc(f)) != EOF)
	 return ((b1 << 8) | b2);

   return EOF;
}



/* pack_mgetl:
 *  Reads a 32 bit long from a file, using motorola byte-ordering.
 */
long pack_mgetl(PACKFILE *f)
{
   int b1, b2, b3, b4;

   if ((b1 = pack_getc(f)) != EOF)
      if ((b2 = pack_getc(f)) != EOF)
	 if ((b3 = pack_getc(f)) != EOF)
	    if ((b4 = pack_getc(f)) != EOF)
	       return (((long)b1 << 24) | ((long)b2 << 16) |
		       ((long)b3 << 8) | (long)b4);

   return EOF;
}



/* pack_mputw:
 *  Writes a 16 bit int to a file, using motorola byte-ordering.
 */
int pack_mputw(int w, PACKFILE *f)
{
   int b1, b2;

   b1 = (w & 0xff00) >> 8;
   b2 = w & 0x00ff;

   if (pack_putc(b1,f)==b1)
      if (pack_putc(b2,f)==b2)
	 return w;

   return EOF;
}



/* pack_mputl:
 *  Writes a 32 bit long to a file, using motorola byte-ordering.
 */
long pack_mputl(long l, PACKFILE *f)
{
   int b1, b2, b3, b4;

   b1 = (int)((l & 0xff000000L) >> 24);
   b2 = (int)((l & 0x00ff0000L) >> 16);
   b3 = (int)((l & 0x0000ff00L) >> 8);
   b4 = (int)l & 0x00ff;

   if (pack_putc(b1,f)==b1)
      if (pack_putc(b2,f)==b2)
	 if (pack_putc(b3,f)==b3)
	    if (pack_putc(b4,f)==b4)
	       return l;

   return EOF;
}



/* pack_fread:
 *  Reads n bytes from f and stores them at memory location p. Returns the 
 *  number of items read, which will be less than n if EOF is reached or an 
 *  error occurs. Error codes are stored in errno.
 */
long pack_fread(void *p, long n, PACKFILE *f)
{
   long c;                       /* counter of bytes read */
   int i;
   unsigned char *cp = (unsigned char *)p;

   for (c=0; c<n; c++) {
      if (--(f->buf_size) > 0)
	 *(cp++) = *(f->buf_pos++);
      else {
	 i = _sort_out_getc(f);
	 if (i == EOF)
	    return c;
	 else
	    *(cp++) = i;
      }
   }

   return n;
}



/* pack_fwrite:
 *  Writes n bytes to the file f from memory location p. Returns the number 
 *  of items written, which will be less than n if an error occurs. Error 
 *  codes are stored in errno.
 */
long pack_fwrite(void *p, long n, PACKFILE *f)
{
   long c;                       /* counter of bytes written */
   unsigned char *cp = (unsigned char *)p;

   for (c=0; c<n; c++) {
      if (++(f->buf_size) >= F_BUF_SIZE) {
	 if (_sort_out_putc(*cp,f) != *cp)
	    return c;
	 cp++;
      }
      else
	 *(f->buf_pos++)=*(cp++);
   }

   return n;
}



/* pack_fgets:
 *  Reads a line from a text file, storing it at location p. Stops when a
 *  linefeed is encountered, or max characters have been read. Returns a
 *  pointer to where it stored the text, or NULL on error. The end of line
 *  is handled by detecting '\n' characters: '\r' is simply ignored.
 */
char *pack_fgets(char *p, int max, PACKFILE *f)
{
   int c;

   if (pack_feof(f)) {
      p[0] = 0;
      return NULL;
   }

   for (c=0; c<max-1; c++) {
      p[c] = pack_getc(f);
      if (p[c] == '\r')
	 c--;
      else if (p[c] == '\n')
	 break;
   }

   p[c] = 0;

   if (errno)
      return NULL;
   else
      return p;
}



/* pack_fputs:
 *  Writes a string to a text file, returning zero on success, -1 on error.
 *  EOL ('\n') characters are expanded to CR/LF ('\r\n') pairs.
 */
int pack_fputs(char *p, PACKFILE *f)
{
   while (*p) {
      if (*p == '\n') {
	 pack_putc('\r', f);
	 pack_putc('\n', f);
      }
      else
	 pack_putc(*p, f);

      p++;
   }

   if (errno)
      return -1;
   else
      return 0;
}



/* _sort_out_getc:
 *  Helper function for the pack_getc() macro.
 */
int _sort_out_getc(PACKFILE *f)
{
   if (f->buf_size == 0) {
      if (f->todo <= 0)
	 f->eof = TRUE;
      return *(f->buf_pos++);
   }
   return refill_buffer(f);
}



/* refill_buffer:
 *  Refills the read buffer. The file must have been opened in read mode,
 *  and the buffer must be empty.
 */
static int refill_buffer(PACKFILE *f)
{
   int sz;

   if (f->eof || (f->todo <= 0)) {     /* EOF */
      f->eof = TRUE;
      return EOF;
   }

   /* else refill the buffer */

   if (f->pack) {
      UNPACK_DATA *dat = (UNPACK_DATA *)f->pack_data;
      f->buf_size = pack_read(dat, F_BUF_SIZE, f->buf);
      if (dat->f->eof)
	 f->todo = 0;
      if (dat->f->error)
	 goto err;
   }
   else {
      f->buf_size = (int)((F_BUF_SIZE < f->todo) ? F_BUF_SIZE : f->todo);
      f->todo -= f->buf_size;

      if (_dos_read(f->hndl, f->buf, f->buf_size, &sz) != 0)
	 goto err;

      if (sz != f->buf_size)
	 goto err;
   }

   f->buf_pos = f->buf;
   f->buf_size--;
   if (f->buf_size <= 0)
      if (f->todo <= 0)
	 f->eof = TRUE;

   return *(f->buf_pos++);

   err:
   errno=EFAULT;
   f->error = TRUE;
   return EOF;
}



/* _sort_out_putc:
 *  Helper function for the pack_putc() macro.
 */
int _sort_out_putc(int c, PACKFILE *f)
{
   f->buf_size--;

   if (flush_buffer(f,FALSE))
      return EOF;

   f->buf_size++;
   return (*(f->buf_pos++)=c);
}



/* flush_buffer:
 * flushes a file buffer to the disk. The file must be open in write mode.
 */
static int flush_buffer(PACKFILE *f, int last)
{
   int sz;

   if (f->buf_size > 0) {
      if (f->pack) {
	 if (pack_write((PACK_DATA *)f->pack_data, f->buf_size, f->buf,last))
	    goto err;
      }
      else { 
	 if (_dos_write(f->hndl, f->buf, f->buf_size, &sz) != 0)
	    goto err;

	 if (sz != f->buf_size)
	    goto err;
      }
   }
   f->buf_pos=f->buf;
   f->buf_size = 0;
   return 0;

   err:
   errno=EFAULT;
   f->error = TRUE;
   return EOF;
}



/***************************************************
 ************ LZSS compression routines ************
 ***************************************************

   This compression algorithm is based on the ideas of Lempel and Ziv,
   with the modifications suggested by Storer and Szymanski. The algorithm 
   is based on the use of a ring buffer, which initially contains zeros. 
   We read several characters from the file into the buffer, and then 
   search the buffer for the longest string that matches the characters 
   just read, and output the length and position of the match in the buffer.

   With a buffer size of 4096 bytes, the position can be encoded in 12
   bits. If we represent the match length in four bits, the <position,
   length> pair is two bytes long. If the longest match is no more than
   two characters, then we send just one character without encoding, and
   restart the process with the next letter. We must send one extra bit
   each time to tell the decoder whether we are sending a <position,
   length> pair or an unencoded character, and these flags are stored as
   an eight bit mask every eight items.

   This implementation uses binary trees to speed up the search for the
   longest match.

   Original code by Haruhiko Okumura, 4/6/1989.
   12-2-404 Green Heights, 580 Nagasawa, Yokosuka 239, Japan.

   Modified for use in the Allegro filesystem by Shawn Hargreaves.

   Use, distribute, and modify this code freely.
*/



/* pack_inittree:
 *  For i = 0 to N-1, rson[i] and lson[i] will be the right and left 
 *  children of node i. These nodes need not be initialized. Also, dad[i] 
 *  is the parent of node i. These are initialized to N, which stands for 
 *  'not used.' For i = 0 to 255, rson[N+i+1] is the root of the tree for 
 *  strings that begin with character i. These are initialized to N. Note 
 *  there are 256 trees.
 */
static void pack_inittree(PACK_DATA *dat)
{
   int i;

   for (i=N+1; i<=N+256; i++)
      dat->rson[i] = N;

   for (i=0; i<N; i++)
      dat->dad[i] = N;
}



/* pack_insertnode:
 *  Inserts a string of length F, text_buf[r..r+F-1], into one of the trees 
 *  (text_buf[r]'th tree) and returns the longest-match position and length 
 *  via match_position and match_length. If match_length = F, then removes 
 *  the old node in favor of the new one, because the old one will be 
 *  deleted sooner. Note r plays double role, as tree node and position in 
 *  the buffer. 
 */
static void pack_insertnode(int r, PACK_DATA *dat)
{
   int i, p, cmp;
   unsigned char *key;
   unsigned char *text_buf = dat->text_buf;

   cmp = 1;
   key = &text_buf[r];
   p = N + 1 + key[0];
   dat->rson[r] = dat->lson[r] = N;
   dat->match_length = 0;

   for (;;) {

      if (cmp >= 0) {
	 if (dat->rson[p] != N)
	    p = dat->rson[p];
	 else {
	    dat->rson[p] = r;
	    dat->dad[r] = p;
	    return;
	 }
      }
      else {
	 if (dat->lson[p] != N)
	    p = dat->lson[p];
	 else {
	    dat->lson[p] = r;
	    dat->dad[r] = p;
	    return;
	 }
      }

      for (i = 1; i < F; i++)
	 if ((cmp = key[i] - text_buf[p + i]) != 0)
	    break;

      if (i > dat->match_length) {
	 dat->match_position = p;
	 if ((dat->match_length = i) >= F)
	    break;
      }
   }

   dat->dad[r] = dat->dad[p];
   dat->lson[r] = dat->lson[p];
   dat->rson[r] = dat->rson[p];
   dat->dad[dat->lson[p]] = r;
   dat->dad[dat->rson[p]] = r;
   if (dat->rson[dat->dad[p]] == p)
      dat->rson[dat->dad[p]] = r;
   else
      dat->lson[dat->dad[p]] = r;
   dat->dad[p] = N;                 /* remove p */
}



/* pack_deletenode:
 *  Removes a node from a tree.
 */
static void pack_deletenode(int p, PACK_DATA *dat)
{
   int q;

   if (dat->dad[p] == N)
      return;     /* not in tree */

   if (dat->rson[p] == N)
      q = dat->lson[p];
   else
      if (dat->lson[p] == N)
	 q = dat->rson[p];
      else {
	 q = dat->lson[p];
	 if (dat->rson[q] != N) {
	    do {
	       q = dat->rson[q];
	    } while (dat->rson[q] != N);
	    dat->rson[dat->dad[q]] = dat->lson[q];
	    dat->dad[dat->lson[q]] = dat->dad[q];
	    dat->lson[q] = dat->lson[p];
	    dat->dad[dat->lson[p]] = q;
	 }
	 dat->rson[q] = dat->rson[p];
	 dat->dad[dat->rson[p]] = q;
      }

   dat->dad[q] = dat->dad[p];
   if (dat->rson[dat->dad[p]] == p)
      dat->rson[dat->dad[p]] = q;
   else
      dat->lson[dat->dad[p]] = q;

   dat->dad[p] = N;
}



/* pack_write:
 *  Called by flush_buffer(). Packs size bytes from buf, using the pack 
 *  information contained in dat. Returns 0 on success.
 */
static int pack_write(PACK_DATA *dat, int size, unsigned char *buf, int last)
{
   int i = dat->i;
   int c = dat->c;
   int len = dat->len;
   int r = dat->r;
   int s = dat->s;
   int last_match_length = dat->last_match_length;
   int code_buf_ptr = dat->code_buf_ptr;
   unsigned char mask = dat->mask;
   int ret = 0;

   if (dat->state==2)
      goto pos2;
   else
      if (dat->state==1)
	 goto pos1;

   dat->code_buf[0] = 0;
      /* code_buf[1..16] saves eight units of code, and code_buf[0] works
	 as eight flags, "1" representing that the unit is an unencoded
	 letter (1 byte), "0" a position-and-length pair (2 bytes).
	 Thus, eight units require at most 16 bytes of code. */

   code_buf_ptr = mask = 1;

   s = 0;
   r = N - F;
   pack_inittree(dat);

   for (len=0; (len < F) && (size > 0); len++) {
      dat->text_buf[r+len] = *(buf++);
      if (--size == 0) {
	 if (!last) {
	    dat->state = 1;
	    goto getout;
	 }
      }
      pos1:
	 ; 
   }

   if (len == 0)
      goto getout;

   for (i=1; i <= F; i++)
      pack_insertnode(r-i,dat);
	    /* Insert the F strings, each of which begins with one or
	       more 'space' characters. Note the order in which these
	       strings are inserted. This way, degenerate trees will be
	       less likely to occur. */
   pack_insertnode(r,dat);
	    /* Finally, insert the whole string just read. match_length
	       and match_position are set. */

   do {
      if (dat->match_length > len)
	 dat->match_length = len;  /* match_length may be long near the end */

      if (dat->match_length <= THRESHOLD) {
	 dat->match_length = 1;  /* Not long enough match.  Send one byte. */
	 dat->code_buf[0] |= mask;    /* 'send one byte' flag */
	 dat->code_buf[code_buf_ptr++] = dat->text_buf[r]; /* Send uncoded. */
      }
      else {
	 /* Send position and length pair. Note match_length > THRESHOLD. */
	 dat->code_buf[code_buf_ptr++] = (unsigned char) dat->match_position;
	 dat->code_buf[code_buf_ptr++] = (unsigned char)
				     (((dat->match_position >> 4) & 0xf0) |
				      (dat->match_length - (THRESHOLD + 1)));
      }

      if ((mask <<= 1) == 0) {               /* Shift mask left one bit. */
	 for (i=0; i < code_buf_ptr; i++)    /* Send at most 8 units of */
	    pack_putc(dat->code_buf[i], dat->f);  /* code together */
	    if (pack_ferror(dat->f)) {
	       ret = EOF;
	       goto getout;
	    }
	    dat->code_buf[0] = 0;
	    code_buf_ptr = mask = 1;
      }

      last_match_length = dat->match_length;

      for (i=0; (i < last_match_length) && (size > 0); i++) {
	 c = *(buf++);
	 if (--size == 0) {
	    if (!last) {
	       dat->state = 2;
	       goto getout;
	    }
	 }
	 pos2:
	 pack_deletenode(s,dat);    /* Delete old strings and */
	 dat->text_buf[s] = c;      /* read new bytes */
	 if (s < F-1)
	    dat->text_buf[s+N] = c; /* If the position is near the end of
				       buffer, extend the buffer to make
				       string comparison easier. */
	 s = (s+1) & (N-1);
	 r = (r+1) & (N-1);         /* Since this is a ring buffer,
				       increment the position modulo N. */
	 pack_insertnode(r,dat);    /* Register the string in
				       text_buf[r..r+F-1] */
      }

      while (i++ < last_match_length) {   /* After the end of text, */
	 pack_deletenode(s,dat);          /* no need to read, but */
	 s = (s+1) & (N-1);
	 r = (r+1) & (N-1);
	 if (--len)
	    pack_insertnode(r,dat);       /* buffer may not be empty. */
      }

   } while (len > 0);   /* until length of string to be processed is zero */

   if (code_buf_ptr > 1) {         /* Send remaining code. */
      for (i=0; i < code_buf_ptr; i++) {
	 pack_putc(dat->code_buf[i], dat->f);
	 if (pack_ferror(dat->f)) {
	    ret = EOF;
	    goto getout;
	 }
      }
   }

   dat->state = 0;

   getout:

   dat->i = i;
   dat->c = c;
   dat->len = len;
   dat->r = r;
   dat->s = s;
   dat->last_match_length = last_match_length;
   dat->code_buf_ptr = code_buf_ptr;
   dat->mask = mask;

   return ret;
}



/* pack_read:
 *  Called by refill_buffer(). Unpacks from dat into buf, until either
 *  EOF is reached or s bytes have been extracted. Returns the number of
 *  bytes added to the buffer
 */
static int pack_read(UNPACK_DATA *dat, int s, unsigned char *buf)
{
   int i = dat->i;
   int j = dat->j;
   int k = dat->k;
   int r = dat->r;
   int c = dat->c;
   unsigned int flags = dat->flags;
   int size = 0;

   if (dat->state==2)
      goto pos2;
   else
      if (dat->state==1)
	 goto pos1;

   r = N-F;
   flags = 0;

   for ( ; ; ) {

      if (((flags >>= 1) & 256) == 0) {
	 if ((c = pack_getc(dat->f)) == EOF)
	    break;
	 flags = c | 0xff00;        /* uses higher byte to count eight */
      }

      if (flags & 1) {
	 if ((c = pack_getc(dat->f)) == EOF)
	    break;
	 dat->text_buf[r++] = c;
	 r &= (N - 1);
	 *(buf++) = c;
	 if (++size >= s) {
	    dat->state = 1;
	    goto getout;
	 }
	 pos1:
	    ;     /* MWC insists on this */
      }
      else {
	 if ((i = pack_getc(dat->f)) == EOF)
	    break;
	 if ((j = pack_getc(dat->f)) == EOF)
	    break;
	 i |= ((j & 0xf0) << 4);
	 j = (j & 0x0f) + THRESHOLD;
	 for (k=0; k <= j; k++) {
	    c = dat->text_buf[(i + k) & (N - 1)];
	    dat->text_buf[r++] = c;
	    r &= (N - 1);
	    *(buf++) = c;
	    if (++size >= s) {
	       dat->state = 2;
	       goto getout;
	    }
	    pos2:
	       ;     /* MWC didn't like the label without this */
	 }
      }
   }

   dat->state = 0;

   getout:

   dat->i = i;
   dat->j = j;
   dat->k = k;
   dat->r = r;
   dat->c = c;
   dat->flags = flags;

   return size;
}


