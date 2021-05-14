/*
		  //  /     /     ,----  ,----.  ,----.  ,----.
		/ /  /     /     /      /    /  /    /  /    /
	      /  /  /     /     /___   /       /____/  /    /
	    /---/  /     /     /      /  __   /\      /    /
	  /    /  /     /     /      /    /  /  \    /    /
	/     /  /____ /____ /____  /____/  /    \  /____/

	Low Level Game Routines (version 1.0)

	File I/O and compression routines.

	See allegro.txt for instructions and copyright conditions.

	By Shawn Hargreaves,
	1 Salisbury Road,
	Market Drayton,
	Shropshire,
	England TF9 1AJ
	email slh100@tower.york.ac.uk (until 1996)
*/

#include "errno.h"
#include "dos.h"
#include "io.h"
#include "string.h"
#include "stdlib.h"

#include "allegro.h"

#define lmalloc(x)      malloc((int)x)

#define ENMFILES        ENMFILE
#define EREAD           EFAULT
#define EWRITE          EFAULT

#define Fsetdta(x);             /* don't need this! */

#ifdef BORLAND

#define DMABUFFER               struct find_t
#define Fsfirst(name,attrib)    _dos_findfirst(name,attrib,&dta)
#define Fsnext()                _dos_findnext(&dta)
#define Fclose(f)               _rtl_close(f)
#define Fopen(name,mode)        _rtl_open(name,mode)
#define Fcreate(name,mode)      _rtl_creat(name,mode)
#define Fread(f,size,buf)       _rtl_read(f,buf,(short)size)
#define Fwrite(f,size,buf)      _rtl_write(f,buf,(short)size)

#else    /* must be GCC */

#include <dir.h> 
#include <fcntl.h> 
#include <sys/stat.h>
#include <osfcn.h>

#define DMABUFFER               struct ffblk
#define Fsfirst(name,attrib)    findfirst(name,(struct ffblk *)&dta,attrib)
#define Fsnext()                findnext((struct ffblk *)&dta)
#define Fclose(f)               close(f)
#define Fopen(name,mode)        open(name,O_RDONLY|O_BINARY,S_IREAD|S_IWRITE)
#define Fcreate(name,mode)      open(name,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,S_IREAD|S_IWRITE)
#define Fread(f,size,buf)       read(f,buf,(int)size)
#define Fwrite(f,size,buf)      write(f,buf,(int)size)

#endif   /* GCC */


#define N            4096     /* 4k buffers for LZ compression */
#define F            18       /* upper limit for LZ match length */
#define THRESHOLD    2        /* LZ encode string into position and length
				 if match length is greater than this */


typedef struct PACK_DATA            /* stuff for doing LZ compression */
{
   FILE *f;                         /* the 'real' file */
   short state;                       /* where have we got to in the pack? */
   short i, c, len, r, s;
   short last_match_length, code_buf_ptr;
   unsigned char mask;
   char code_buf[17];
   short match_position;
   short match_length;
   short lson[N+1];                   /* left children, */
   short rson[N+257];                 /* right children, */
   short dad[N+1];                    /* and parents, = binary search trees */
   unsigned char text_buf[N+F-1];   /* ring buffer, with F-1 extra bytes
				       for string comparison */
} PACK_DATA;


typedef struct UNPACK_DATA          /* for reading LZ files */
{
   FILE *f;                         /* the 'real' file */
   short state;                       /* where have we got to? */
   short i, j, k, r, c;
   unsigned short flags;
   unsigned char text_buf[N+F-1];   /* ring buffer, with F-1 extra bytes
				       for string comparison */
} UNPACK_DATA;


short _refill_buffer(FILE *f);
short _flush_buffer(FILE *f, short last);
void _pack_inittree(PACK_DATA *dat);
void _pack_insertnode(short r, PACK_DATA *dat);
void _pack_deletenode(short p, PACK_DATA *dat);
short _pack_write(PACK_DATA *dat, short size, unsigned char *buf, short last);
short _pack_read(UNPACK_DATA *dat, short s, unsigned char *buf);
void _load_pc_data(char *pos, long size, FILE *f);
void _load_st_data(char *pos, long size, FILE *f);
void _load_sprite(SPRITE *s, short bits, FILE *f);



char *get_filename(path)
register char *path;
{
   register short pos;

   for (pos=0; path[pos]; pos++)
      ;

   while ((pos>0) && (path[pos-1] != '\\') && (path[pos-1] != '/'))
      pos--;

   return path+pos;
}



char *get_extension(filename)
register char *filename;
{
   register short pos, end;

   for (end=0; filename[end]; end++)
      ;

   pos = end;

   while ((pos>0) && (filename[pos-1] != '.') &&
	  (filename[pos-1] != '\\') && (filename[pos-1] != '/'))
      pos--;

   if (filename[pos-1] == '.')
      return filename+pos;

   return filename+end;
}



void put_backslash(filename)
char *filename;
{
   short i = strlen(filename);

   if (i>0)
      if ((filename[i-1]=='\\') || (filename[i-1]=='/'))
	 return;

   filename[i++] = '\\';
   filename[i] = 0;
}



short file_exists(filename, attrib, aret)
char *filename;
short attrib;
short *aret;
{
   DMABUFFER dta;

   Fsetdta(&dta);
   errno = Fsfirst(filename,attrib);
   if (aret)
#ifdef GCC
      *aret = dta.ff_attrib;
#else
      *aret = dta.attrib;
#endif
   Fsetdta(old_dta);
   return ((errno) ? 0 : -1);
}



long file_size(filename)
char *filename;
{
   DMABUFFER dta;

   Fsetdta(&dta);
   errno = Fsfirst(filename, F_RDONLY | F_HIDDEN | F_ARCH);
   Fsetdta(old_dta);
   if (errno)
      return 0L;

#ifdef GCC
   return dta.ff_fsize;
#else
   return dta.size;
#endif
}



short delete_file(filename)
char *filename;
{
   unlink(filename);
   return errno;
}



short for_each_file(name,attrib,call_back,param)
char *name;
short attrib;
void (*call_back)();
short param;
{
   char buf[80];
   short c = 0;
   DMABUFFER dta;

   Fsetdta(&dta);

   errno = Fsfirst(name,attrib);
   if (errno!=0) {
      Fsetdta(old_dta);
      return 0;
   }

   do {
      strcpy(buf,name);
#ifdef GCC
      strcpy(get_filename(buf),dta.ff_name);
      (*call_back)(buf,dta.ff_attrib,param);
#else
      strcpy(get_filename(buf),dta.name);
      (*call_back)(buf,dta.attrib,param);
#endif
      if (errno!=0)
	 break;
      c++;
   } while ((errno=Fsnext())==0);

#ifndef GCC
   if (errno==ENMFILES)
#endif
      errno=0;

   Fsetdta(old_dta);
   return c;
}



FILE *_my_fopen(filename, mode)
char *filename;
char *mode;
{
   register FILE *f, *f2;
   DMABUFFER dta;
   register short c;
   long header = FALSE;

   errno=0;

   if ((f=malloc(sizeof(FILE)))==(FILE *)NULL) {
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
	 PACK_DATA *dat = malloc(sizeof(PACK_DATA));
	 if (!dat) {
	    errno = ENOMEM;
	    free(f);
	    return NULL;
	 }
	 if ((dat->f = _my_fopen(filename,F_WRITE)) == (FILE *)NULL) {
	    free(dat);
	    free(f);
	    return NULL;
	 }
	 putl(F_PACK_MAGIC,dat->f); /* can't fail (buffer not full yet) */
	 for (c=0; c < N - F; c++)
	    dat->text_buf[c] = 0;   /* clear the buffer */
	 dat->state = 0;
	 f->pack_data = (char *)dat;
      }
      else {
	 f->pack_data = NULL;
	 if ((f->hndl = Fcreate(filename, 0)) < 0) {
	    errno = f->hndl;
	    free(f);
	    return NULL;
	 }
	 errno = 0;
      }
      f->todo = 0L;
      if (header)
	 putl(F_NOPACK_MAGIC,f);    /* can't fail (buffer not full yet) */
   }

   else {         /* must be a read */
      if (f->pack) {
	 UNPACK_DATA *dat = malloc(sizeof(UNPACK_DATA));
	 if (!dat) {
	    errno = ENOMEM;
	    free(f);
	    return NULL;
	 }
	 if ((dat->f = _my_fopen(filename,F_READ)) == (FILE *)NULL) {
	    free(dat);
	    free(f);
	    return NULL;
	 }
	 header = getl(dat->f);
	 if (header == F_PACK_MAGIC) {
	    for (c=0; c < N - F; c++)
	       dat->text_buf[c] = 0;   /* clear the buffer */
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
	       fclose(dat->f);
	       free(dat);
	       free(f);
	       if (errno==0)
		  errno = EDOM;
	       return NULL;
	    }
	 }
      }
      else {
	 f->pack_data = NULL;
	 Fsetdta(&dta);
	 errno = Fsfirst(filename, 0x23);
	 if (errno != 0) {
	    free(f);
	    Fsetdta(old_dta);
	    return NULL;
	 }
#ifdef GCC
	 f->todo = dta.ff_fsize;
#else
	 f->todo = dta.size;
#endif
	 if ((f->hndl = Fopen(filename, 0)) < 0) {
	    errno = f->hndl;
	    free(f);
	    return NULL;
	 }
      }
   }
   return f;
}



short _my_fclose(f)
FILE *f;
{
   if (f) {
      if (f->write)
	 _flush_buffer(f,TRUE);

      if (f->pack_data) {
	 if (f->write) {
	    PACK_DATA *dat = (PACK_DATA *)f->pack_data;
	    _my_fclose(dat->f);
	 }
	 else {
	    UNPACK_DATA *dat = (UNPACK_DATA *)f->pack_data;
	    _my_fclose(dat->f);
	 }
	 free(f->pack_data);
      }
      else
	 Fclose(f->hndl);

      free(f);
      return errno;
   }

   return 0;
}



short _sort_out_getc(f)
register FILE *f;
{
   if (f->buf_size == 0) {
      if (f->todo <= 0)
	 f->eof = TRUE;
      return *(f->buf_pos++);
   }
   return _refill_buffer(f);
}



short _my_getw(f)
register FILE *f;
{
   register short b1, b2;

   if ((b1 = getc(f)) != EOF)
      if ((b2 = getc(f)) != EOF)
	 return ((b1 << 8) | b2);

   return EOF;
}



short _my_igetw(f)
register FILE *f;
{
   register short b1, b2;

   if ((b1 = getc(f)) != EOF)
      if ((b2 = getc(f)) != EOF)
	 return ((b2 << 8) | b1);

   return EOF;
}



long _my_getl(f)
register FILE *f;
{
   register short b1, b2, b3, b4;

   if ((b1 = getc(f)) != EOF)
      if ((b2 = getc(f)) != EOF)
	 if ((b3 = getc(f)) != EOF)
	    if ((b4 = getc(f)) != EOF)
	       return (((long)b1 << 24) | ((long)b2 << 16) |
		       ((long)b3 << 8) | (long)b4);

   return EOF;
}



long _my_igetl(f)
register FILE *f;
{
   register short b1, b2, b3, b4;

   if ((b1 = getc(f)) != EOF)
      if ((b2 = getc(f)) != EOF)
	 if ((b3 = getc(f)) != EOF)
	    if ((b4 = getc(f)) != EOF)
	       return (((long)b4 << 24) | ((long)b3 << 16) |
		       ((long)b2 << 8) | (long)b1);

   return EOF;
}



short _sort_out_putc(c,f)
register short c;
register FILE *f;
{
   f->buf_size--;

   if (_flush_buffer(f,FALSE))
      return EOF;

   f->buf_size++;
   return (*(f->buf_pos++)=c);
}



short _my_putw(w,f)
register short w;
register FILE *f;
{
   register short b1, b2;

   b1 = (w & 0xff00) >> 8;
   b2 = w & 0x00ff;

   if (putc(b1,f)==b1)
      if (putc(b2,f)==b2)
	 return w;

   return EOF;
}



short _my_iputw(w,f)
register short w;
register FILE *f;
{
   register short b1, b2;

   b1 = (w & 0xff00) >> 8;
   b2 = w & 0x00ff;

   if (putc(b2,f)==b2)
      if (putc(b1,f)==b1)
	 return w;

   return EOF;
}



long _my_putl(l,f)
register long l;
register FILE *f;
{
   register short b1, b2, b3, b4;

   b1 = (short)((l & 0xff000000L) >> 24);
   b2 = (short)((l & 0x00ff0000L) >> 16);
   b3 = (short)((l & 0x0000ff00L) >> 8);
   b4 = (short)l & 0x00ff;

   if (putc(b1,f)==b1)
      if (putc(b2,f)==b2)
	 if (putc(b3,f)==b3)
	    if (putc(b4,f)==b4)
	       return l;

   return EOF;
}



long _my_fread(p,n,f)
register void *p;
register long n;
register FILE *f;
{
   register long c;                       /* counter of bytes read */
   register short i;
   register unsigned char *cp = (unsigned char *)p;

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



long _my_fwrite(p,n,f)
register void *p;
register long n;
register FILE *f;
{
   register long c;                       /* counter of bytes written */
   register unsigned char *cp = (unsigned char *)p;

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



short _refill_buffer(f)
register FILE *f;
{
   /* refills the read buffer. The file MUST have been opened in read mode,
      and the buffer must be empty. */

   if (f->eof || (f->todo <= 0)) {     /* EOF */
      f->eof = TRUE;
      return EOF;
   }

   /* else refill the buffer */

   if (f->pack) {
      UNPACK_DATA *dat = (UNPACK_DATA *)f->pack_data;
      f->buf_size = _pack_read(dat, F_BUF_SIZE, f->buf);
      if (dat->f->eof)
	 f->todo = 0;
      if (dat->f->error)
	 goto err;
   }
   else {
      f->buf_size = (short)((F_BUF_SIZE < f->todo) ? F_BUF_SIZE : f->todo);
      f->todo -= f->buf_size;
      if (Fread(f->hndl, (long)f->buf_size, f->buf) != (long)f->buf_size)
	 goto err;
   }

   f->buf_pos = f->buf;
   f->buf_size--;
   if (f->buf_size <= 0)
      if (f->todo <= 0)
	 f->eof = TRUE;

   return *(f->buf_pos++);

   err:
   errno=EREAD;
   f->error = TRUE;
   return EOF;
}



short _flush_buffer(f,last)
register FILE *f;
short last;
{
   /* flushes a file buffer to the disk */

   if (f->buf_size > 0) {
      if (f->pack) {
	 if (_pack_write((PACK_DATA *)f->pack_data, f->buf_size, f->buf,last))
	    goto err;
      }
      else { 
	 if (Fwrite(f->hndl, (long)f->buf_size, f->buf) != (long)f->buf_size)
	    goto err;
      }
   }
   f->buf_pos=f->buf;
   f->buf_size = 0;
   return 0;

   err:
   errno=EWRITE;
   f->error = TRUE;
   return EOF;
}


/***************************************************/
/************ LZSS compression routines ************/
/***************************************************/

/*
   This compression algorithm is based on the ideas of Lempel and Ziv,
   with the modifications suggested by Storer and Szymanski. The method
   of using a binary tree was proposed by Bell. The algorithm is based
   on the use of a ring buffer, which initially contains zeros. We read
   several characters from the file into the buffer, and then search the
   buffer for the longest string that matches the characters just read,
   and output the length and position of the match in the buffer.

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



void _pack_inittree(dat)
register PACK_DATA *dat;
{
   /* For i = 0 to N-1, rson[i] and lson[i] will be the right and
      left children of node i. These nodes need not be initialized.
      Also, dad[i] is the parent of node i. These are initialized to
      N, which stands for 'not used.'
      For i = 0 to 255, rson[N+i+1] is the root of the tree
      for strings that begin with character i. These are initialized
      to N. Note there are 256 trees. */

   register short i;

   for (i=N+1; i<=N+256; i++)
      dat->rson[i] = N;

   for (i=0; i<N; i++)
      dat->dad[i] = N;
}



void _pack_insertnode(r,dat)
register short r;
register PACK_DATA *dat;
{
   /* Inserts string of length F, text_buf[r..r+F-1], into one
      of the trees (text_buf[r]'th tree) and returns the longest-match
      position and length via match_position and match_length. If
      match_length = F, then removes the old node in favor of the new
      one, because the old one will be deleted sooner. Note r plays
      double role, as tree node and position in buffer. */

   register short i, p, cmp;
   register unsigned char *key;
   register unsigned char *text_buf = dat->text_buf;

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



void _pack_deletenode(p,dat)
register short p;
register PACK_DATA *dat;
{
   register short q;

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



short _pack_write(dat,size,buf,last)
register PACK_DATA *dat;
register short size;
register unsigned char *buf;
short last;
{
   /* called by _flush_buffer(). Packs size bytes from buf, using
      the pack information contained in dat. Returns 0 on success. */

   register short i = dat->i;
   register short c = dat->c;
   register short len = dat->len;
   short r = dat->r;
   register short s = dat->s;
   short last_match_length = dat->last_match_length;
   short code_buf_ptr = dat->code_buf_ptr;
   unsigned char mask = dat->mask;
   short ret = 0;

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
   _pack_inittree(dat);

   for (len=0; (len < F) && (size > 0); len++) {
      dat->text_buf[r+len] = *(buf++);
      if (--size == 0) {
	 if (!last) {
	    dat->state = 1;
	    goto getout;
	 }
      }
      pos1:
	 ;     /* MWC got upset when this wasn't there */
   }

   if (len == 0)
      goto getout;

   for (i=1; i <= F; i++)
      _pack_insertnode(r-i,dat);
	    /* Insert the F strings, each of which begins with one or
	       more 'space' characters. Note the order in which these
	       strings are inserted. This way, degenerate trees will be
	       less likely to occur. */
   _pack_insertnode(r,dat);
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
	    putc(dat->code_buf[i], dat->f);  /* code together */
	    if (ferror(dat->f)) {
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
	 _pack_deletenode(s,dat);   /* Delete old strings and */
	 dat->text_buf[s] = c;      /* read new bytes */
	 if (s < F-1)
	    dat->text_buf[s+N] = c; /* If the position is near the end of
				       buffer, extend the buffer to make
				       string comparison easier. */
	 s = (s+1) & (N-1);
	 r = (r+1) & (N-1);         /* Since this is a ring buffer,
				       increment the position modulo N. */
	 _pack_insertnode(r,dat);   /* Register the string in
				       text_buf[r..r+F-1] */
      }

      while (i++ < last_match_length) {   /* After the end of text, */
	 _pack_deletenode(s,dat);         /* no need to read, but */
	 s = (s+1) & (N-1);
	 r = (r+1) & (N-1);
	 if (--len)
	    _pack_insertnode(r,dat);      /* buffer may not be empty. */
      }

   } while (len > 0);   /* until length of string to be processed is zero */

   if (code_buf_ptr > 1) {         /* Send remaining code. */
      for (i=0; i < code_buf_ptr; i++) {
	 putc(dat->code_buf[i], dat->f);
	 if (ferror(dat->f)) {
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



short _pack_read(dat,s,buf)
register UNPACK_DATA *dat;
short s;
register unsigned char *buf;
{
   /* called by refill_buffer(). Unpacks from dat into buf, until either
      EOF is reached or s bytes have been extracted. Returns the number of
      bytes added to the buffer */

   register short i = dat->i;
   register short j = dat->j;
   register short k = dat->k;
   register short r = dat->r;
   register short c = dat->c;
   unsigned short flags = dat->flags;
   short size = 0;

   if (dat->state==2)
      goto pos2;
   else
      if (dat->state==1)
	 goto pos1;

   r = N-F;
   flags = 0;

   for ( ; ; ) {

      if (((flags >>= 1) & 256) == 0) {
	 if ((c = getc(dat->f)) == EOF)
	    break;
	 flags = c | 0xff00;        /* uses higher byte to count eight */
      }

      if (flags & 1) {
	 if ((c = getc(dat->f)) == EOF)
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
	 if ((i = getc(dat->f)) == EOF)
	    break;
	 if ((j = getc(dat->f)) == EOF)
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



/*********************************************************/
/************ data file manipulation routines ************/
/*********************************************************/


void _load_st_data(pos, size, f)
char *pos;
long size;
FILE *f;
{
   short c;
   unsigned short d1, d2, d3, d4;

   size /= 8;           /* number of 4 word planes to read */

   while (size) {
      d1 = getw(f);
      d2 = getw(f);
      d3 = getw(f);
      d4 = getw(f);
      for (c=0; c<16; c++) {
	 *(pos++) = ((d1 & 0x8000) >> 15) + ((d2 & 0x8000) >> 14) +
		    ((d3 & 0x8000) >> 13) + ((d4 & 0x8000) >> 12);
	 d1 <<= 1;
	 d2 <<= 1;
	 d3 <<= 1;
	 d4 <<= 1; 
      }
      size--;
   }
}



void _load_pc_data(pos, size, f)
char *pos;
long size;
FILE *f;
{
   _fread((unsigned char *)pos, size, f);
}



void _load_sprite(s, bits, f)
SPRITE *s;
short bits;
FILE *f;
{
   if (bits==8)
      _load_pc_data(s->dat, (long)s->w*(long)s->h, f);
   else
      _load_st_data(s->dat, (long)s->w/2*(long)s->h, f);
}



DATAFILE *load_datafile(filename)
char *filename;
{
   FILE *f;
   short c, c2;
   short w, h;
   short size;
   DATAFILE *dat;
   short flags;
   RGB rgb;

   f = fopen(filename, F_READ_PACKED);
   if (!f)
      return NULL;

   if (getl(f) != DAT_MAGIC) {
      fclose(f);
      errno = EDOM;
      return NULL;
   }

   size = getw(f);
   if (errno) {
      fclose(f);
      return NULL;
   }

   dat = malloc(sizeof(DATAFILE)*(size+1));
   if (!dat) {
      fclose(f);
      errno = ENOMEM;
      return NULL;
   }

   for (c=0; c<=size; c++) {
      dat[c].type = DAT_END;
      dat[c].size = 0;
      dat[c].dat = NULL;
   }

   for (c=0; c<size; c++) {

      dat[c].type = getw(f);

      switch(dat[c].type) {

      case DAT_DATA:
	 dat[c].size = getl(f);
	 dat[c].dat = lmalloc(dat[c].size);
	 if (!dat[c].dat) {
	    errno = ENOMEM;
	    break;
	 }
	 _fread(dat[c].dat, dat[c].size, f);
	 if (errno) {
	    free(dat[c].dat);
	    dat[c].dat = NULL;
	 }
	 break;

      case DAT_FONT: 
	 dat[c].dat = malloc(sizeof(FONT));
	 if (!dat[c].dat) {
	    errno = ENOMEM;
	    break;
	 }
	 _fread(dat[c].dat, sizeof(FONT), f);
	 if (errno) {
	    free(dat[c].dat);
	    dat[c].dat = NULL;
	 }
	 break;

      case DAT_BITMAP_16: 
	 w = getw(f);
	 h = getw(f);
	 dat[c].dat = create_bitmap(w,h);
	 if (!dat[c].dat) {
	    errno = ENOMEM;
	    break;
	 }
	 _load_st_data(((BITMAP *)dat[c].dat)->line[0], (long)w/2*(long)h, f);
	 if (errno) {
	    destroy_bitmap(dat[c].dat);
	    dat[c].dat = NULL;
	 }
	 break;

      case DAT_BITMAP_256:
	 w = getw(f);
	 h = getw(f);
	 dat[c].dat = create_bitmap(w,h);
	 if (!dat[c].dat) {
	    errno = ENOMEM;
	    break;
	 }
	 _load_pc_data(((BITMAP *)dat[c].dat)->line[0], (long)w*(long)h, f);
	 if (errno) {
	    destroy_bitmap(dat[c].dat);
	    dat[c].dat = NULL;
	 }
	 break;

      case DAT_SPRITE_16:
	 flags = getw(f);
	 w = getw(f);
	 h = getw(f);
	 dat[c].dat = create_sprite(flags,w,h);
	 if (!dat[c].dat) {
	    errno = ENOMEM;
	    break;
	 }
	 _load_sprite(dat[c].dat, 4, f);
	 if (errno) {
	    destroy_sprite(dat[c].dat);
	    dat[c].dat = NULL;
	 }
	 break;

      case DAT_SPRITE_256: 
	 flags = getw(f);
	 w = getw(f);
	 h = getw(f);
	 dat[c].dat = create_sprite(flags,w,h);
	 if (!dat[c].dat) {
	    errno = ENOMEM;
	    break;
	 }
	 _load_sprite(dat[c].dat, 8, f);
	 if (errno) {
	    destroy_sprite(dat[c].dat);
	    dat[c].dat = NULL;
	 }
	 break;

      case DAT_PALLETE_16: 
	 dat[c].dat = malloc(sizeof(PALLETE));
	 if (!dat[c].dat) {
	    errno = ENOMEM;
	    break;
	 }
	 for (c2=0; c2<16; c2++) {
	    rgb.r = getc(f);
	    rgb.g = getc(f);
	    rgb.b = getc(f);
	    ((_RGB *)dat[c].dat)[c2] = rgbtopal(rgb);
	 }
	 for (c2=16; c2<256; c2++)
	    ((_RGB *)dat[c].dat)[c2] = ((_RGB *)dat[c].dat)[c2&0x0f];
	 if (errno) {
	    free(dat[c].dat);
	    dat[c].dat = NULL;
	 }
	 break;

      case DAT_PALLETE_256:
	 dat[c].dat = malloc(sizeof(PALLETE));
	 if (!dat[c].dat) {
	    errno = ENOMEM;
	    break;
	 }
	 for (c2=0; c2<PAL_SIZE; c2++) {
	    rgb.r = getc(f);
	    rgb.g = getc(f);
	    rgb.b = getc(f);
	    ((_RGB *)dat[c].dat)[c2] = rgbtopal(rgb);
	 }
	 if (errno) {
	    free(dat[c].dat);
	    dat[c].dat = NULL;
	 }
	 break;
      }

      if (errno) {
	 dat[c].type = DAT_END;
	 unload_datafile(dat);
	 fclose(f);
	 return NULL;
      }
   }

   fclose(f);
   return dat;
}



void unload_datafile(dat)
DATAFILE *dat;
{
   DATAFILE *p = dat;

   if (!dat)
      return;

   while (p->type != DAT_END) {

      switch (p->type) {

      case DAT_DATA:
      case DAT_FONT:
      case DAT_PALLETE_16:
      case DAT_PALLETE_256:
	 free(p->dat);
	 break;

      case DAT_BITMAP_16:
      case DAT_BITMAP_256:
	 destroy_bitmap(p->dat);
	 break;

      case DAT_SPRITE_16:
      case DAT_SPRITE_256:
	 destroy_sprite(p->dat);
	 break;
      }

      p++;
   }

   free(dat);
}

