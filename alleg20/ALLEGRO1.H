/* 
 *    These definitions will help you to recompile programs written for 
 *    older (1.x) versions of Allegro. Programs that rely on things which
 *    no longer exist (such as SPRITE structs) should include this file 
 *    instead of allegro.h. Things still won't be 100% the same, but this 
 *    will conceal a lot of the differences. On the other hand, the changes 
 *    are (hopefully :-) all improvements, so you might be better off just 
 *    updating any old code you have...
 */


#ifndef ALLEGRO1_H
#define ALLEGRO1_H

#include "allegro.h"

#define allegro_init()              {  allegro_init(); \
				       install_mouse(); \
				       install_timer(); \
				       set_gfx_mode(GFX_VGA, 320, 200, 0, 0); \
				    }

#define TICKS_PER_SECOND            1000

#define _RGB                        RGB
#define rgbtopal(rgb)               rgb
#define paltorgb(rgb)               rgb

#define SPRITE                      BITMAP

#define SPRITE_OPAQUE               0
#define SPRITE_MASKED               1

#define drawsprite                  draw_sprite
#define textmode                    text_mode
#define create_sprite(flags, w, h)  create_bitmap(w, h)
#define destroy_sprite(sprite)      destroy_bitmap(sprite)
#define get_sprite(spr, bmp, x, y)  blit(bmp, spr, x, y, 0, 0, spr->w, spr->h);
#define show(b)                     blit(b, screen, 0, 0, 0, 0, 
							 SCREEN_W, SCREEN_H)

#define do_dialog(dialog)           do_dialog(dialog, -1)
#define popup_dialog(dialog)        popup_dialog(dialog, -1)
#define file_select(msg, path)      file_select(msg, path, NULL)

#define FILE                        PACKFILE

#define F_RDONLY                    0x01 
#define F_HIDDEN                    0x02
#define F_SYSTEM                    0x04
#define F_VOLUME                    0x08
#define F_SUBDIR                    0x10
#define F_ARCH                      0x20 

#define getc(f)                     pack_getc(f)
#define putc(c,f)                   pack_putc(c, f)
#define fopen(name,mode)            pack_fopen(name,mode)
#define fclose(f)                   pack_fclose(f)
#define feof(f)                     pack_feof(f)
#define ferror(f)                   pack_ferror(f)
#define fgetc(f)                    pack_getc(f)
#define getw(f)                     pack_mgetw(f)
#define igetw(f)                    pack_igetw(f)
#define getl(f)                     pack_mgetl(f)
#define igetl(f)                    pack_igetl(f)
#define fputc(c,f)                  pack_putc(c,f)
#define putw(w,f)                   pack_mputw(w,f)
#define iputw(w,f)                  pack_iputw(w,f)
#define putl(l,f)                   pack_mputl(l,f)
#define iputl(l,f)                  pack_iputl(l,f)
#define _fread(p,n,f)               pack_fread(p,n,f)
#define fread(p,size,n,f)           (pack_fread(p,size*n,f) / size)
#define _fwrite(p,n,f)              pack_fwrite(p,n,f)
#define fwrite(p,size,n,f)          (pack_fwrite(p,size*n,f) / size)


#endif          /* ifndef ALLEGRO1_H */

