/* 
 *    This is a little hack I wrote to simplify producing the documentation
 *    in both ASCII and HTML formats. It converts from a weird ascii+markers
 *    format I designed myself (the _tx files), and automates a few things
 *    like constructing indexes and splitting HTML files into multiple parts.
 *    It's not pretty, elegant, or well written, and is liable to get upset 
 *    if it doesn't like the input data. If you change the _tx files, run 
 *    'make docs' to rebuild the text and HTML versions of everything.
 *
 *    Anyone fancy adding TexInfo output to this? That would be cool...
 */


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>


#define TEXT_FLAG             1
#define HTML_FLAG             2
#define HEADING_FLAG          4
#define DEFINITION_FLAG       8
#define CONTINUE_FLAG         16
#define TOC_FLAG              32
#define NO_EOL_FLAG           64
#define MULTIFILE_FLAG        128
#define NOCONTENT_FLAG        256


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
} TOC;


TOC *tochead = NULL;
TOC *toctail = NULL;


int flags = TEXT_FLAG | HTML_FLAG;

char fileheader[256] = "";
char filefooter[256] = "";



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



void add_toc_line(char *buf, int root)
{
   int len;
   TOC *toc;

   len = strlen(buf);
   toc = malloc(sizeof(TOC));
   toc->text = malloc(len+1);
   strcpy(toc->text, buf);
   toc->next = NULL;
   toc->root = root;

   if (toctail) 
      toctail->next = toc;
   else
      tochead = toc;

   toctail = toc;
}



void add_toc(char *buf, int root)
{
   char b[256], b2[256];
   int c, d;
   int done;

   if (root) {
      add_toc_line(buf, 1);
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
	       add_toc_line(b, 0);
	       strcpy(b2, buf+c+d+1);
	       sprintf(buf+c, "<a name=\"%s\">%s</a>%s", b, b, b2);
	       done = 0;
	       break;
	    }
	 }
      } while (!done);
   }
}



int read_file(char *filename)
{
   char buf[1024];
   char *p;
   FILE *f;

   printf("reading %s\n", filename);

   f = fopen(filename, "r");
   if (!f)
      return 1;

   while (fgets(buf, 1023, f)) {
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
	 else if (stricmp(buf+1, "multiplefiles") == 0)
	    flags |= MULTIFILE_FLAG;
	 else if (stricmp(buf+1, "headingnocontent") == 0)
	    flags |= (HEADING_FLAG | NOCONTENT_FLAG);
	 else if (stricmp(buf+1, "heading") == 0)
	    flags |= HEADING_FLAG;
	 else if (stricmp(buf+1, "contents") == 0)
	    add_line("", TOC_FLAG);
	 else if ((tolower(buf[1]=='h')) && (buf[2]=='='))
	    strcpy(fileheader, buf+3);
	 else if ((tolower(buf[1]=='f')) && (buf[2]=='='))
	    strcpy(filefooter, buf+3);
	 else if (buf[1] == '<')
	    add_line(buf+1, (flags | HTML_FLAG | NO_EOL_FLAG) & (~TEXT_FLAG));
	 else if (buf[1] == '@') {
	    add_toc(buf+2, 0);
	    add_line(buf+2, flags | DEFINITION_FLAG);
	 }
	 else if (buf[1] == '\\') {
	    add_toc(buf+2, 0);
	    add_line(buf+2, flags | DEFINITION_FLAG | CONTINUE_FLAG);
	 }
      }
      else {
	 /* some actual text */
	 if (flags & HEADING_FLAG)
	    add_toc(buf, 1);

	 add_line(buf, flags);

	 flags &= ~(HEADING_FLAG | NOCONTENT_FLAG);
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
	 if (toc->root) {
	    if (body) {
	       fprintf(f, "<li><a href=\"#%s_toc\">%s</a>\n", toc->text, toc->text);
	    }
	    else {
	       strcpy(name, filename);
	       sprintf(extension(name)-1, "%c.html", 'a'+section_number);
	       s = name+strlen(name);
	       while ((s > name) && (*(s-1) != '\\') && (*(s-1) != '/'))
		  s--;
	       fprintf(f, "<li><a href=\"%s\">%s</a>\n", s, toc->text);
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
	       fprintf(f, "<a name=\"%s_toc\">", toc->text);

	    fprintf(f, "<li><a href=\"#%s\">%s</a>\n", toc->text, toc->text);

	    if (!nested)
	       fprintf(f, "</a>");

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
	    fprintf(f, "<li><a href=\"#%s\">%s</a>\n", toc->text, toc->text);
	    toc = toc->next;
	 }

	 fprintf(f, "</h4></ul>\n<p><br><br>\n");
      }
   }
}



int write_txt(char *filename)
{
   LINE *line = head;
   char *p;
   int c, len;
   FILE *f;

   printf("writing %s\n", filename);

   f = fopen(filename, "w");
   if (!f)
      return 1;

   while (line) {
      if (line->flags & TEXT_FLAG) {
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
   int i;

   if (s[0]) {
      for (i=0; s[i]; i++) {
	 if (s[i] == '#')
	    fputs(strip_html(t), f);
	 else
	    fputc(s[i], f);
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
	       sprintf(extension(name)-1, "%c.html", 'a'+section_number-1);
	       printf("writing %s\n", name);
	       f = fopen(name, "w");
	       if (!f)
		  return 1;
	       output_headfooter(f, fileheader, line->text);
	    }
	    fprintf(f, "<h1>%s</h1>\n", line->text);
	    empty_count = 0;
	    if ((flags & MULTIFILE_FLAG) && 
		(!(line->flags & NOCONTENT_FLAG)) &&
		(section_number > 0)) {
	       output_toc(f, filename, 0, 1, section_number);
	    }
	    section_number++;
	 }
	 else if (line->flags & DEFINITION_FLAG) {
	    /* output a function definition */
	    fprintf(f, "<b>%s</b>", line->text);
	    if (!(line->flags & CONTINUE_FLAG))
	       fputs("<br>", f);
	    fputs("\n", f);
	    empty_count = 0;
	 }
	 else if ((!(line->flags & NO_EOL_FLAG)) && 
		  (is_empty(strip_html(line->text)))) {
	    /* output an empty line */
	    if (empty_count)
	       fprintf(f, "<br>%s\n", line->text);
	    else
	       fprintf(f, "<p>%s\n", line->text);
	    empty_count++;
	 }
	 else {
	    /* output a normal line */
	    fputs(line->text, f);
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



int main(int argc, char *argv[])
{
   char txtname[256];
   char htmlname[256];
   int err = 0;

   if (argc < 2) {
      printf("Usage: makedoc <input file>\n");
      return 1;
   }

   strcpy(txtname, argv[1]);
   strcpy(extension(txtname), "txt");

   strcpy(htmlname, argv[1]);
   strcpy(extension(htmlname), "html");

   if (read_file(argv[1]) != 0) {
      printf("Error reading input file\n");
      err = 1;
      goto getout;
   }

   if (write_txt(txtname) != 0) {
      printf("Error writing ASCII output file\n");
      err = 1;
      goto getout;
   }

   if (write_html(htmlname) != 0) {
      printf("Error writing HTML output file\n");
      err = 1;
      goto getout;
   }

   getout:

   free_data();

   return err;
}

