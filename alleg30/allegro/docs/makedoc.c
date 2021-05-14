/* 
 *    This is a little hack I wrote to simplify producing the documentation 
 *    in various formats (ASCII, HTML, and TexInfo). It converts from a weird 
 *    text+markers format that I designed myself (the _tx files), and 
 *    automates a few things like constructing indexes and splitting HTML 
 *    files into multiple parts. It's not pretty, elegant, or well written, 
 *    and is liable to get upset if it doesn't like the input data. If you 
 *    change the _tx files, run 'make docs' to rebuild everything.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>


#define TEXT_FLAG                 0x1
#define HTML_FLAG                 0x2
#define HTML_CMD_FLAG             0x4
#define TEXINFO_FLAG              0x8
#define TEXINFO_CMD_FLAG         0x10
#define HEADING_FLAG             0x20
#define DEFINITION_FLAG          0x40
#define CONTINUE_FLAG            0x80
#define TOC_FLAG                0x100
#define INDEX_FLAG              0x200
#define NO_EOL_FLAG             0x400
#define MULTIFILE_FLAG          0x800
#define NOCONTENT_FLAG         0x1000
#define NO_INDENT_FLAG         0x2000
#define NODE_FLAG              0x4000
#define NONODE_FLAG            0x8000
#define STARTOUTPUT_FLAG      0x10000
#define ENDOUTPUT_FLAG        0x20000


typedef struct LINE
{
   char *text;
   struct LINE *next;
   int flags;
} LINE;


LINE *head = NULL;
LINE *tail = NULL;


typedef struct TOC
{
   char *text;
   struct TOC *next;
   int root;
   int texinfoable;
   int htmlable;
} TOC;


TOC *tochead = NULL;
TOC *toctail = NULL;


int flags = TEXT_FLAG | HTML_FLAG | TEXINFO_FLAG;

int nostrip = 0;
int strip_indent = -1;
int last_toc_line = INT_MIN;

char fileheader[256] = "";
char filefooter[256] = "";

char *html_extension = "html";



char *extension(char *filename)
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



char *get_filename(char *path)
{
   int pos;

   for (pos=0; path[pos]; pos++)
      ; /* do nothing */ 

   while ((pos>0) && (path[pos-1] != '\\') && (path[pos-1] != '/'))
      pos--;

   return path+pos;
}



int is_empty(char *s)
{
   while (*s) {
      if (*s != ' ')
	 return 0;

      s++;
   }

   return 1;
}



void free_data()
{
   LINE *line = head;
   LINE *prev;

   TOC *tocline = tochead;
   TOC *tocprev;

   while (line) {
      prev = line;
      line = line->next;

      free(prev->text);
      free(prev);
   }

   while (tocline) {
      tocprev = tocline;
      tocline = tocline->next;

      free(tocprev->text);
      free(tocprev);
   }
}



void add_line(char *buf, int flags)
{
   int len;
   LINE *line;

   len = strlen(buf);
   line = malloc(sizeof(LINE));
   line->text = malloc(len+1);
   strcpy(line->text, buf);
   line->next = NULL;
   line->flags = flags;

   if (tail)
      tail->next = line;
   else
      head = line;

   tail = line;
}



void add_toc_line(char *buf, int root, int num, int texinfoable, int htmlable)
{
   int len;
   TOC *toc;

   len = strlen(buf);
   toc = malloc(sizeof(TOC));
   toc->text = malloc(len+1);
   strcpy(toc->text, buf);
   toc->next = NULL;
   toc->root = root;
   toc->texinfoable = ((texinfoable) && (num > last_toc_line+1));
   toc->htmlable = htmlable;

   last_toc_line = num;

   if (toctail) 
      toctail->next = toc;
   else
      tochead = toc;

   toctail = toc;
}



void add_toc(char *buf, int root, int num, int texinfoable, int htmlable)
{
   char b[256], b2[256];
   int c, d;
   int done;

   if (root) {
      add_toc_line(buf, 1, num, texinfoable, htmlable);
      strcpy(b, buf);
      sprintf(buf, "<a name=\"%s\">%s</a>", b, b);
   }
   else {
      do {
	 done = 1;
	 for (c=0; buf[c]; c++) {
	    if (buf[c] == '@') {
	       for (d=0; isalnum(buf[c+d+1]) || (buf[c+d+1] == '_'); d++)
		  b[d] = buf[c+d+1];
	       b[d] = 0;
	       add_toc_line(b, 0, num, texinfoable, htmlable);
	       strcpy(b2, buf+c+d+1);
	       sprintf(buf+c, "<a name=\"%s\">%s</a>%s", b, b, b2);
	       done = 0;
	       break;
	    }
	 }
      } while (!done);
   }
}



char *my_fgets(char *p, int max, FILE *f)
{
   int c, ch;

   if (feof(f)) {
      p[0] = 0;
      return NULL;
   }

   for (c=0; c<max-1; c++) {
      ch = getc(f);
      if (ch == EOF)
	 break;
      p[c] = ch;
      if (p[c] == '\r')
	 c--;
      else if (p[c] == '\n')
	 break;
      else if (p[c] == '\t') {
	 p[c] = ' ';
	 while ((c+1) & 7) {
	    c++;
	    p[c] = ' ';
	 }
      }
   }

   p[c] = 0;
   return p;
}



int strincmp(char *s1, char *s2, int n)
{
   while (n) {
      if (tolower(*s1) != tolower(*s2))
	 return 1;

      if (!*s1)
	 return 0;

      s1++;
      s2++;
      n--;
   }

   return 0;
}



int read_file(char *filename)
{
   char buf[1024];
   char *p;
   FILE *f;
   int line = 0;

   printf("reading %s\n", filename);

   f = fopen(filename, "r");
   if (!f)
      return 1;

   while (my_fgets(buf, 1023, f)) {
      line++;

      /* strip EOL */
      p = buf+strlen(buf)-1;
      while ((p >= buf) && ((*p == '\r') || (*p == '\n'))) {
	 *p = 0;
	 p--;
      }

      if (buf[0] == '@') {
	 /* a marker line */
	 if (stricmp(buf+1, "text") == 0)
	    flags |= TEXT_FLAG;
	 else if (stricmp(buf+1, "!text") == 0)
	    flags &= ~TEXT_FLAG;
	 else if (stricmp(buf+1, "html") == 0)
	    flags |= HTML_FLAG;
	 else if (stricmp(buf+1, "!html") == 0)
	    flags &= ~HTML_FLAG;
	 else if (stricmp(buf+1, "texinfo") == 0)
	    flags |= TEXINFO_FLAG;
	 else if (stricmp(buf+1, "!texinfo") == 0)
	    flags &= ~TEXINFO_FLAG;
	 else if (stricmp(buf+1, "indent") == 0)
	    flags &= ~NO_INDENT_FLAG;
	 else if (stricmp(buf+1, "!indent") == 0)
	    flags |= NO_INDENT_FLAG;
	 else if (stricmp(buf+1, "multiplefiles") == 0)
	    flags |= MULTIFILE_FLAG;
	 else if (stricmp(buf+1, "headingnocontent") == 0)
	    flags |= (HEADING_FLAG | NOCONTENT_FLAG);
	 else if (stricmp(buf+1, "heading") == 0)
	    flags |= HEADING_FLAG;
	 else if (stricmp(buf+1, "contents") == 0)
	    add_line("", TOC_FLAG);
	 else if (stricmp(buf+1, "index") == 0)
	    add_line("", INDEX_FLAG);
	 else if (strincmp(buf+1, "node ", 5) == 0) {
	    add_toc_line(buf+6, 0, line, 1, 0);
	    add_line(buf+6, NODE_FLAG);
	 }
	 else if (strincmp(buf+1, "hnode ", 6) == 0) {
	    add_toc_line(buf+7, 0, line, 1, 0);
	    add_line(buf+7, NODE_FLAG | HTML_FLAG);
	 }
	 else if (strincmp(buf+1, "nonode", 6) == 0)
	    flags |= NONODE_FLAG;
	 else if (strincmp(buf+1, "startoutput ", 12) == 0)
	    add_line(buf+13, STARTOUTPUT_FLAG);
	 else if (strincmp(buf+1, "endoutput ", 10) == 0)
	    add_line(buf+11, ENDOUTPUT_FLAG);
	 else if ((tolower(buf[1]=='h')) && (buf[2]=='='))
	    strcpy(fileheader, buf+3);
	 else if ((tolower(buf[1]=='f')) && (buf[2]=='='))
	    strcpy(filefooter, buf+3);
	 else if (buf[1] == '<')
	    add_line(buf+1, (flags | HTML_FLAG | HTML_CMD_FLAG | NO_EOL_FLAG ) & (~TEXT_FLAG));
	 else if (buf[1] == '$')
	    add_line(buf+2, TEXINFO_FLAG | TEXINFO_CMD_FLAG);
	 else if (buf[1] == '@') {
	    add_toc(buf+2, 0, line, !(flags & NONODE_FLAG), 1);
	    add_line(buf+2, flags | DEFINITION_FLAG);
	    flags &= ~NONODE_FLAG;
	 }
	 else if (buf[1] == '\\') {
	    add_toc(buf+2, 0, line, !(flags & NONODE_FLAG), 1);
	    add_line(buf+2, flags | DEFINITION_FLAG | CONTINUE_FLAG);
	    flags &= ~NONODE_FLAG;
	 }
	 else if (buf[1] == '#') {
	    /* comment */
	 }
	 else {
	    fclose(f);
	    fprintf(stderr, "Unknown token '%s'\n", buf);
	    return 1; 
	 }
      }
      else {
	 /* some actual text */
	 if (flags & HEADING_FLAG)
	    add_toc(buf, 1, line, !(flags & NONODE_FLAG), (flags & HTML_FLAG));

	 add_line(buf, flags);

	 flags &= ~(HEADING_FLAG | NOCONTENT_FLAG | NONODE_FLAG);
      }
   }

   fclose(f);
   return 0;
}



char *strip_html(char *p)
{
   static char buf[256];
   int c;

   c = 0;
   while (*p) {
      if (*p == '<') {
	 while ((*p) && (*p != '>'))
	    p++;
	 if (*p)
	    p++;
      }
      else {
	 buf[c] = *p;
	 c++;
	 p++;
      }
   } 

   buf[c] = 0;

   return buf;
}



/* DOS -> Windows extended character set mapping */
unsigned char charmap[128] =
{
   0xC7, 0xFC, 0xE9, 0xE2, 0xE4, 0xE0, 0xE5, 0xE7,
   0xEA, 0xEB, 0xE8, 0xEF, 0xEE, 0xEC, 0xC4, 0xC5,
   0xC9, 0xE6, 0xC6, 0xF4, 0xF6, 0xF2, 0xFB, 0xF9,
   0xFF, 0xD6, 0xDC, 0xA2, 0xA3, 0xA5, '^',  '^',
   0xE1, 0xED, 0xF3, 0xFA, 0xF1, 0xD1, 0xAA, 0xBA,
   0xBF, '^',  0xAC, 0xBD, 0xBC, 0xA1, 0xAB, 0xBB,
   '^',  '^',  '^',  '^',  '^',  '^',  '^',  '^',
   '^',  '^',  '^',  '^',  '^',  '^',  '^',  '^',
   '^',  '^',  '^',  '^',  '^',  '^',  '^',  '^',
   '^',  '^',  '^',  '^',  '^',  '^',  '^',  '^',
   '^',  '^',  '^',  '^',  '^',  '^',  '^',  '^',
   '^',  '^',  '^',  '^',  '^',  '^',  '^',  '^',
   '^',  '^',  '^',  '^',  '^',  '^',  0xB5, '^',
   '^',  0xD8, '^',  '^',  '^',  '^',  '^',  '^',
   '^',  0xB1, '^',  '^',  '^',  '^',  0xF7, '^',
   0xB0, 0xB7, 0xB7, '^',  '^',  0xB2, '^',  '^'
};



void hfputs(char *string, FILE *file)
{
   int c;

   while (*string) {
      if ((string[0] == '&') && 
	  ((string[1] == 'l') || (string[1] == 'g')) &&
	  (string[2] == 't')) {

	 fputc('&', file);
	 fputc(string[1], file);
	 fputc('t', file);
	 fputc(';', file);

	 string += 3;
      }
      else { 
	 c = (unsigned char)*string;

	 if (c >= 128)
	    fputc(charmap[c-128], file);
	 else 
	    fputc(c, file);

	 string++;
      }
   }
}



void hfprintf(FILE *file, char *format, ...)
{
   char buf[1024];

   va_list ap;
   va_start(ap, format);
   vsprintf(buf, format, ap);
   va_end(ap);

   hfputs(buf, file);
}



void output_toc(FILE *f, char *filename, int root, int body, int part)
{
   char name[256];
   char *s;
   TOC *toc = tochead;
   int nested = 0;
   int section_number = 0;

   if (root) {
      toc = tochead;
      if (toc)
	 toc = toc->next;

      fprintf(f, "<ul><h4>\n");

      while (toc) {
	 if ((toc->root) && (toc->htmlable)) {
	    if (body) {
	       hfprintf(f, "<li><a href=\"#%s_toc\">%s</a>\n", toc->text, toc->text);
	    }
	    else {
	       strcpy(name, filename);
	       s = extension(name)-1;
	       if ((int)s - (int)get_filename(name) > 5)
		  s = get_filename(name)+5;
	       sprintf(s, "%03d.%s", section_number, html_extension);
	       hfprintf(f, "<li><a href=\"%s\">%s</a>\n", get_filename(name), toc->text);
	    }
	    section_number++;
	 }
	 toc = toc->next;
      }

      fprintf(f, "</h4></ul><p>\n");
   }

   if (body) {
      toc = tochead;
      if (toc)
	 toc = toc->next;

      if (part <= 0) {
	 fprintf(f, "<ul><h2>\n");

	 while (toc) {
	    if (toc->htmlable) {
	       if (!toc->root) {
		  if (!nested) {
		     fprintf(f, "<ul><h4>\n");
		     nested = 1;
		  }
	       }
	       else {
		  if (nested) {
		     fprintf(f, "</h4></ul><p>\n");
		     nested = 0;
		  }
	       }

	       if (!nested)
		  hfprintf(f, "<a name=\"%s_toc\">", toc->text);

	       hfprintf(f, "<li><a href=\"#%s\">%s</a>\n", toc->text, toc->text);

	       if (!nested)
		  fprintf(f, "</a>");
	    }

	    toc = toc->next;
	 }

	 if (nested)
	    fprintf(f, "</ul>\n");

	 fprintf(f, "</h2></ul>\n");
      }
      else {
	 section_number = 0;
	 fprintf(f, "<p>\n<ul><h4>\n");

	 while ((toc) && (section_number < part)) {
	    if (toc->root)
	       section_number++;
	    toc = toc->next;
	 }

	 while ((toc) && (!toc->root)) {
	    if (toc->htmlable)
	       hfprintf(f, "<li><a href=\"#%s\">%s</a>\n", toc->text, toc->text);
	    toc = toc->next;
	 }

	 fprintf(f, "</h4></ul>\n<p><br><br>\n");
      }
   }
}



char *first_word(char *s)
{
   static char buf[80];
   int i;

   for (i=0; s[i] && s[i] != ' '; i++)
      buf[i] = s[i];

   buf[i] = 0;
   return buf;
}



int scmp(const void *e1, const void *e2)
{
   char *s1 = *((char **)e1);
   char *s2 = *((char **)e2);

   return stricmp(s1, s2);
}



void output_texinfo_toc(FILE *f, int root, int body, int part)
{
   #define TOC_SIZE     8192

   TOC *toc = tochead;
   int section_number = 0;
   char **ptr;
   char *s;
   int i, j;

   fprintf(f, "@menu\n");

   if (root) {
      toc = tochead;
      if (toc)
	 toc = toc->next;

      while (toc) {
	 if ((toc->root) && (toc->texinfoable)) {
	    s = first_word(toc->text);
	    fprintf(f, "* %s::", s);
	    for (i=strlen(s); i<24; i++)
	       fputc(' ', f);
	    fprintf(f, "%s\n", toc->text);
	 }
	 toc = toc->next;
      }
   }

   if (body) {
      toc = tochead;
      if (toc)
	 toc = toc->next;

      if (part <= 0) {
	 ptr = malloc(TOC_SIZE * sizeof(char *));
	 i = 0;

	 while (toc) {
	    if ((!toc->root) && (toc->texinfoable) && (i < TOC_SIZE))
	       ptr[i++] = toc->text;

	    toc = toc->next;
	 }

	 if (i > 1)
	    qsort(ptr, i, sizeof(char *), scmp);

	 for (j=0; j<i; j++)
	    fprintf(f, "* %s::\n", ptr[j]);

	 free(ptr);
      }
      else {
	 section_number = 0;

	 while ((toc) && (section_number < part)) {
	    if (toc->root)
	       section_number++;
	    toc = toc->next;
	 }

	 while ((toc) && (!toc->root)) {
	    if (toc->texinfoable)
	       fprintf(f, "* %s::\n", toc->text);
	    toc = toc->next;
	 }
      }
   }

   fprintf(f, "@end menu\n");
}



int valid_texinfo_node(char *s, char **next, char **prev)
{
   TOC *toc = tochead;

   *next = *prev = "";

   while (toc) {
      if ((!toc->root) && (toc->texinfoable)) {
	 if (stricmp(toc->text, s) == 0) {
	    do {
	       toc = toc->next;
	    } while ((toc) && ((!toc->texinfoable) || (toc->root)));

	    if (toc)
	       *next = toc->text;

	    return 1;
	 }

	 *prev = toc->text;
      }

      toc = toc->next;
   }

   return 0;
}



char *node_name(int i)
{
   TOC *toc = tochead;

   toc = tochead;
   if (toc)
      toc = toc->next;

   while (toc) {
      if (toc->root) {
	 i--;
	 if (!i)
	    return first_word(toc->text);
      }

      toc = toc->next;
   }

   return "";
}



int write_txt(char *filename, int partial)
{
   LINE *line = head;
   char *p;
   int c, len;
   FILE *f;
   int outputting = !partial;

   printf("writing %s\n", filename);

   f = fopen(filename, "w");
   if (!f)
      return 1;

   while (line) {
      if (line->flags & (STARTOUTPUT_FLAG | ENDOUTPUT_FLAG)) {
	 if (stricmp(line->text, get_filename(filename)) == 0) {
	    if (line->flags & STARTOUTPUT_FLAG)
	       outputting = 1;
	    else
	       outputting = 0;
	 }
      }
      else if ((line->flags & TEXT_FLAG) && (outputting)) {
	 p = line->text;

	 if (line->flags & HEADING_FLAG) {
	    /* output a section heading */
	    p = strip_html(p);
	    len = strlen(p);
	    for (c=0; c<len+26; c++)
	       fputc('=', f);
	    fprintf(f, "\n============ %s ============\n", p);
	    for (c=0; c<len+26; c++)
	       fputc('=', f);
	    fputs("\n", f);
	 }
	 else {
	    while (*p) {
	       /* less-than HTML tokens */
	       if ((p[0] == '&') && 
		   (tolower(p[1]) == 'l') && 
		   (tolower(p[2]) == 't')) {
		  fputc('<', f);
		  p += 3;
	       }
	       /* greater-than HTML tokens */
	       else if ((p[0] == '&') && 
			(tolower(p[1]) == 'g') && 
			(tolower(p[2]) == 't')) {
		  fputc('>', f);
		  p += 3;
	       }
	       /* strip other HTML tokens */
	       else if (p[0] == '<') {
		  while ((*p) && (*p != '>'))
		     p++;
		  if (*p)
		     p++;
	       }
	       /* output other characters */
	       else {
		  fputc(*p, f);
		  p++;
	       }
	    }
	    fputs("\n", f);
	 }
      }

      line = line->next;
   }

   fclose(f);
   return 0;
}



void output_headfooter(FILE *f, char *s, char *t)
{
   char buf[256];
   int i, n;

   strcpy(buf, s);

   if (buf[0]) {
      for (i=0; buf[i]; i++) {
	 if (strincmp(buf+i, ".html", 5) == 0) {
	    i++;
	    n = strlen(html_extension);
	    memcpy(buf+i, html_extension, n);
	    if (n < 4)
	       memmove(buf+i+n, buf+i+4, strlen(buf+4)+1);
	    i += n;
	 }
      }

      for (i=0; buf[i]; i++) {
	 if (buf[i] == '#')
	    fputs(strip_html(t), f);
	 else
	    fputc(buf[i], f);
      }
      fputs("\n", f);
   }
}



int write_html(char *filename)
{
   char name[256];
   int empty_count = 0;
   int section_number = 0;
   LINE *line = head;
   FILE *f;
   char *s;

   printf("writing %s\n", filename);

   f = fopen(filename, "w");
   if (!f)
      return 1;

   while (line) {
      if (line->flags & HTML_FLAG) {
	 if (line->flags & HEADING_FLAG) {
	    /* output a section heading, switching file as required */
	    if ((flags & MULTIFILE_FLAG) && (section_number > 0)) {
	       if (section_number > 1)
		  output_headfooter(f, filefooter, "");
	       fclose(f);
	       strcpy(name, filename);
	       s = extension(name)-1;
	       if ((int)s - (int)get_filename(name) > 5)
		  s = get_filename(name)+5;
	       sprintf(s, "%03d.%s", section_number-1, html_extension);
	       printf("writing %s\n", name);
	       f = fopen(name, "w");
	       if (!f)
		  return 1;
	       output_headfooter(f, fileheader, line->text);
	    }
	    hfprintf(f, "<h1>%s</h1>\n", line->text);
	    empty_count = 0;
	    if ((flags & MULTIFILE_FLAG) && 
		(!(line->flags & NOCONTENT_FLAG)) &&
		(section_number > 0)) {
	       output_toc(f, filename, 0, 1, section_number);
	    }
	    section_number++;
	 }
	 else if (line->flags & NODE_FLAG) {
	    /* output a node marker */
	    fprintf(f, "<p><hr><p>\n");
	 }
	 else if (line->flags & DEFINITION_FLAG) {
	    /* output a function definition */
	    hfprintf(f, "<b>%s</b>", line->text);
	    if (!(line->flags & CONTINUE_FLAG))
	       fputs("<br>", f);
	    fputs("\n", f);
	    empty_count = 0;
	 }
	 else if ((!(line->flags & NO_EOL_FLAG)) && 
		  (is_empty(strip_html(line->text)))) {
	    /* output an empty line */
	    if (empty_count)
	       hfprintf(f, "<br>%s\n", line->text);
	    else
	       hfprintf(f, "<p>%s\n", line->text);
	    empty_count++;
	 }
	 else {
	    /* output a normal line */
	    hfputs(line->text, f);
	    fputs("\n", f);
	    empty_count = 0;
	 }
      }
      else if (line->flags & TOC_FLAG) {
	 if (flags & MULTIFILE_FLAG)
	    output_toc(f, filename, 1, 0, -1);
	 else
	    output_toc(f, filename, 1, 1, -1);
      }

      line = line->next;
   }

   if ((flags & MULTIFILE_FLAG) && (section_number > 1))
      output_headfooter(f, filefooter, "");

   fclose(f);
   return 0;
}



void html2texinfo(FILE *f, char *p)
{
   char buf[256];
   int i = 0;

   while ((*p) && (*p != '>'))
      buf[i++] = *(p++);

   buf[i] = 0;

   if (stricmp(buf, "pre") == 0) {
      fputs("@example\n", f);
      nostrip = 1;
      strip_indent = -1;
   }
   else if (stricmp(buf, "/pre") == 0) {
      fputs("@end example\n", f);
      nostrip = 0;
      strip_indent = -1;
   }
   else if (stricmp(buf, "br") == 0) {
      fputs("@*", f);
   }
   else if (stricmp(buf, "hr") == 0) {
      fputs("----------------------------------------------------------------------", f);
   }
   else if (stricmp(buf, "p") == 0) {
      fputs("\n", f);
   }
   else if (stricmp(buf, "ul") == 0) {
      fputs("\n@itemize @bullet\n", f);
   }
   else if (stricmp(buf, "/ul") == 0) {
      fputs("@end itemize\n", f);
   }
   else if (stricmp(buf, "li") == 0) {
      fputs("@item ", f);
   }
}



int write_texinfo(char *filename)
{
   char buf[256];
   LINE *line = head;
   char *p, *str, *next, *prev;
   int in_item = 0;
   int section_number = 0;
   int toc_waiting = 0;
   int i;
   FILE *f;

   printf("writing %s\n", filename);

   f = fopen(filename, "w");
   if (!f)
      return 1;

   while (line) {
      if (line->flags & TEXINFO_FLAG) {
	 p = line->text;

	 /* end of an ftable? */
	 if (in_item) {
	    if ((*p) && (*p != ' ') && (*p != '<')) {
	       fputs("@end ftable\n", f);
	       in_item = 0;
	    }
	 }

	 if (!in_item) {
	    /* start a new node? */
	    str = strstr(p, "<a name=\"");
	    if (str > p) {
	       str += strlen("<a name=\"");
	       for (i=0; str[i] && str[i] != '"'; i++)
		  buf[i] = str[i];
	       buf[i] = 0;

	       if (!(line->flags & NONODE_FLAG) &&
		   (valid_texinfo_node(buf, &next, &prev))) {
		  if (toc_waiting) {
		     output_texinfo_toc(f, 0, 1, section_number-1);
		     toc_waiting = 0;
		  }
		  fprintf(f, "@node %s, %s, %s, %s\n", buf, next, prev, node_name(section_number-1));
	       }

	       fputs("@ftable @asis\n", f);
	       in_item = 1;
	    }
	 }

	 /* munge the indentation to make it look nicer */
	 if (!(line->flags & NO_INDENT_FLAG)) {
	    if (nostrip) {
	       if (strip_indent >= 0) {
		  for (i=0; (i<strip_indent) && (*p) && (isspace(*p)); i++)
		     p++;
	       }
	       else {
		  if (!is_empty(p)) {
		     strip_indent = 0;
		     while ((*p) && (isspace(*p))) {
			strip_indent++;
			p++;
		     }
		  }
	       }
	    }
	    else {
	       while ((*p) && (isspace(*p)))
		  p++;
	    }
	 }

	 if (line->flags & TEXINFO_CMD_FLAG) {
	    /* raw output of texinfo commands */
	    fputs(p, f);
	    fputs("\n", f);
	 }
	 else if (line->flags & HTML_CMD_FLAG) {
	    /* process HTML commands */
	    while (*p) {
	       if (p[0] == '<') {
		  html2texinfo(f, p+1);
		  while ((*p) && (*p != '>'))
		     p++;
		  if (*p)
		     p++;
	       }
	       else
		  p++;
	    }
	 }
	 else if (line->flags & HEADING_FLAG) {
	    /* output a section heading */
	    if (section_number > 0) {
	       if (in_item) {
		  fputs("@end ftable\n", f);
		  in_item = 0;
	       }
	       p = strip_html(p);
	       fprintf(f, "@node %s, ", node_name(section_number));
	       fprintf(f, "%s, ", node_name(section_number+1));
	       fprintf(f, "%s, Top\n", node_name(section_number-1));
	       fprintf(f, "@chapter %s\n", p);
	       if (!(line->flags & NOCONTENT_FLAG))
		  toc_waiting = 1;
	    }
	    section_number++;
	 }
	 else {
	    if (line->flags & DEFINITION_FLAG)
	       fputs("@item ", f);

	    while (*p) {
	       /* less-than HTML tokens */
	       if ((p[0] == '&') && 
		   (tolower(p[1]) == 'l') && 
		   (tolower(p[2]) == 't')) {
		  fputc('<', f);
		  p += 3;
	       }
	       /* greater-than HTML tokens */
	       else if ((p[0] == '&') && 
			(tolower(p[1]) == 'g') && 
			(tolower(p[2]) == 't')) {
		  fputc('>', f);
		  p += 3;
	       }
	       /* process other HTML tokens */
	       else if (p[0] == '<') {
		  html2texinfo(f, p+1);
		  while ((*p) && (*p != '>'))
		     p++;
		  if (*p)
		     p++;
	       }
	       /* output other characters */
	       else {
		  if ((*p == '{') || (*p == '}') || (*p == '@'))
		     fputc('@', f);
		  fputc(*p, f);
		  p++;
	       }
	    }

	    fputs("\n", f);
	 }
      }
      else if (line->flags & NODE_FLAG) {
	 if (in_item) {
	    fputs("@end ftable\n", f);
	    if (toc_waiting) {
	       output_texinfo_toc(f, 0, 1, section_number-1);
	       toc_waiting = 0;
	    }
	 }

	 if (valid_texinfo_node(line->text, &next, &prev))
	    fprintf(f, "@node %s, %s, %s, %s\n", line->text, next, prev, node_name(section_number-1));

	 fputs("@ftable @asis\n", f);
	 in_item = 1;
      }
      else if (line->flags & TOC_FLAG) {
	 output_texinfo_toc(f, 1, 0, 0);
      }
      else if (line->flags & INDEX_FLAG) {
	 output_texinfo_toc(f, 0, 1, 0);
      }

      line = line->next;
   }

   fclose(f);
   return 0;
}



int main(int argc, char *argv[])
{
   char filename[256] = "";
   char txtname[256] = "";
   char htmlname[256] = "";
   char texinfoname[256] = "";
   int err = 0;
   int partial = 0;
   int i;

   for (i=1; i<argc; i++) {
      if ((stricmp(argv[i], "-ascii") == 0) && (i<argc-1)) {
	 strcpy(txtname, argv[++i]);
      }
      else if ((stricmp(argv[i], "-html") == 0) && (i<argc-1)) {
	 strcpy(htmlname, argv[++i]);
	 html_extension = "html";
      }
      else if ((stricmp(argv[i], "-htm") == 0) && (i<argc-1)) {
	 strcpy(htmlname, argv[++i]);
	 html_extension = "htm";
      }
      else if ((stricmp(argv[i], "-texinfo") == 0) && (i<argc-1)) {
	 strcpy(texinfoname, argv[++i]);
      }
      else if (stricmp(argv[i], "-part") == 0) {
	 partial = 1;
      }
      else if (argv[i][0] == '-') {
	 fprintf(stderr, "Unknown option '%s'\n", argv[i]);
	 return 1;
      }
      else {
	 strcpy(filename, argv[i]);
      }
   }

   if (!filename[0]) {
      fprintf(stderr, "Usage: makedoc [-ascii file] [-htm[l] file] [-texinfo file] [-part] infile\n");
      return 1;
   }

   if ((!txtname[0]) && (!htmlname[0]) && (!texinfoname[0])) {
      strcpy(txtname, filename);
      strcpy(extension(txtname), "txt");

      strcpy(htmlname, filename);
      strcpy(extension(htmlname), html_extension);

      strcpy(texinfoname, filename);
      strcpy(extension(texinfoname), "txi");
   }

   if (read_file(filename) != 0) {
      fprintf(stderr, "Error reading input file\n");
      err = 1;
      goto getout;
   }

   if (txtname[0]) {
      if (write_txt(txtname, partial) != 0) {
	 fprintf(stderr, "Error writing ASCII output file\n");
	 err = 1;
	 goto getout;
      }
   }

   if (htmlname[0]) {
      if (write_html(htmlname) != 0) {
	 fprintf(stderr, "Error writing HTML output file\n");
	 err = 1;
	 goto getout;
      }
   }

   if (texinfoname[0]) {
      if (write_texinfo(texinfoname) != 0) {
	 fprintf(stderr, "Error writing TexInfo output file\n");
	 err = 1;
	 goto getout;
      }
   }

   getout:

   free_data();

   return err;
}

