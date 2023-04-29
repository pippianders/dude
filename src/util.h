/* ncdu - NCurses Disk Usage

  Copyright (c) 2007-2022 Yoran Heling

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef _util_h
#define _util_h

#include "global.h"
#include <ncurses.h>


void die(const char *, ...);


/* UI colors: (foreground, background, attrs)
 *  NAME         OFF         DARK                DARK-BG   */
#define UI_COLORS \
  C(DEFAULT,     _,_,0  ,    _,      _,    0,    WHITE,  BLACK,0)\
  C(BOX_TITLE,   _,_,B  ,    BLUE,   _,    B,    BLUE,   BLACK,B)\
  C(HD,          _,_,R  ,    BLACK,  CYAN, 0,    BLACK,  CYAN, 0)    /* header & footer */\
  C(SEL,         _,_,R  ,    WHITE,  GREEN,B,    WHITE,  GREEN,B)\
  C(NUM,         _,_,0  ,    YELLOW, _,    B,    YELLOW, BLACK,B)\
  C(NUM_HD,      _,_,R  ,    YELLOW, CYAN, B,    YELLOW, CYAN, B)\
  C(NUM_SEL,     _,_,R  ,    YELLOW, GREEN,B,    YELLOW, GREEN,B)\
  C(KEY,         _,_,B  ,    YELLOW, _,    B,    YELLOW, BLACK,B)\
  C(KEY_HD,      _,_,B|R,    YELLOW, CYAN, B,    YELLOW, CYAN, B)\
  C(DIR,         _,_,0  ,    BLUE,   _,    B,    BLUE,   BLACK,B)\
  C(DIR_SEL,     _,_,R  ,    BLUE,   GREEN,B,    BLUE,   GREEN,B)\
  C(FLAG,        _,_,0  ,    RED,    _,    0,    RED,    BLACK,0)\
  C(FLAG_SEL,    _,_,R  ,    RED,    GREEN,0,    RED,    GREEN,0)\
  C(GRAPH,       _,_,0  ,    MAGENTA,_,    0,    MAGENTA,BLACK,0)\
  C(GRAPH_SEL,   _,_,R  ,    MAGENTA,GREEN,0,    MAGENTA,GREEN,0)

enum ui_coltype {
#define C(name, ...) UIC_##name,
  UI_COLORS
#undef C
  UIC_NONE
};

/* Color & attribute manipulation */
extern int uic_theme;

void uic_init(void);
void uic_set(enum ui_coltype);


/* updated when window is resized */
extern int winrows, wincols;

/* used by the nc* functions and macros */
extern int subwinr, subwinc;

/* used by formatsize to choose between base 2 or 10 prefixes */
extern int si;


/* Macros for managing struct dir and struct dir_ext */

#define dir_memsize(n)     (offsetof(struct dir, name)+1+strlen(n))
#define dir_ext_offset(n)  ((dir_memsize(n) + 7) & ~7)
#define dir_ext_memsize(n) (dir_ext_offset(n) + sizeof(struct dir_ext))
#define dir_ext_ptr(d)     ((d)->flags & FF_EXT ? (struct dir_ext *) ( ((char *)(d)) + dir_ext_offset((d)->name) ) : NULL)


/* Instead of using several ncurses windows, we only draw to stdscr.
 * the functions nccreate, ncprint and the macros ncaddstr and ncaddch
 * mimic the behaviour of ncurses windows.
 * This works better than using ncurses windows when all windows are
 * created in the correct order: it paints directly on stdscr, so
 * wrefresh, wnoutrefresh and other window-specific functions are not
 * necessary.
 * Also, this method doesn't require any window objects, as you can
 * only create one window at a time.
*/

/* updates winrows, wincols, and displays a warning when the terminal
 * is smaller than the specified minimum size. */
int ncresize(int, int);

/* creates a new centered window with border */
void nccreate(int, int, const char *);

/* printf something somewhere in the last created window */
void ncprint(int, int, const char *, ...);

/* Add a "tab" to a window */
void nctab(int, int, int, const char *);

/* same as the w* functions of ncurses, with a color */
#define ncaddstr(r, c, s) mvaddstr(subwinr+(r), subwinc+(c), s)
#define  ncaddch(r, c, s)  mvaddch(subwinr+(r), subwinc+(c), s)
#define   ncmove(r, c)        move(subwinr+(r), subwinc+(c))

/* add stuff with a color */
#define mvaddstrc(t, r, c, s) do { uic_set(t); mvaddstr(r, c, s); } while(0)
#define  mvaddchc(t, r, c, s) do { uic_set(t);  mvaddch(r, c, s); } while(0)
#define   addstrc(t,       s) do { uic_set(t);   addstr(      s); } while(0)
#define    addchc(t,       s) do { uic_set(t);    addch(      s); } while(0)
#define ncaddstrc(t, r, c, s) do { uic_set(t); ncaddstr(r, c, s); } while(0)
#define  ncaddchc(t, r, c, s) do { uic_set(t);  ncaddch(r, c, s); } while(0)
#define  mvhlinec(t, r, c, s, n) do { uic_set(t);  mvhline(r, c, s, n); } while(0)

/* crops a string into the specified length */
char *cropstr(const char *, int);

/* Converts the given size in bytes into a float (0 <= f < 1000) and a unit string */
float formatsize(int64_t, const char **);

/* print size in the form of xxx.x XB */
void printsize(enum ui_coltype, int64_t);

/* int2string with thousand separators */
char *fullsize(int64_t);

/* format's a file mode as a ls -l string */
char *fmtmode(unsigned short);

/* read locale information from the environment */
void read_locale(void);

/* recursively free()s a directory tree */
void freedir(struct dir *);

/* generates full path from a dir item,
   returned pointer will be overwritten with a subsequent call */
const char *getpath(struct dir *);

/* returns the root element of the given dir struct */
struct dir *getroot(struct dir *);

/* Add two signed 64-bit integers. Returns INT64_MAX if the result would
 * overflow, or 0 if it would be negative. At least one of the integers must be
 * positive.
 * I use uint64_t's to detect the overflow, as (a + b < 0) relies on undefined
 * behaviour, and (INT64_MAX - b >= a) didn't work for some reason. */
#define adds64(a, b) ((a) > 0 && (b) > 0\
    ? ((uint64_t)(a) + (uint64_t)(b) > (uint64_t)INT64_MAX ? INT64_MAX : (a)+(b))\
    : (a)+(b) < 0 ? 0 : (a)+(b))

/* Adds a value to the size, asize and items fields of *d and its parents */
void addparentstats(struct dir *, int64_t, int64_t, uint64_t, int);


/* A simple stack implemented in macros */
#define nstack_init(_s) do {\
    (_s)->size = 10;\
    (_s)->top = 0;\
    (_s)->list = xmalloc(10*sizeof(*(_s)->list));\
  } while(0)

#define nstack_push(_s, _v) do {\
    if((_s)->size <= (_s)->top) {\
      (_s)->size *= 2;\
      (_s)->list = xrealloc((_s)->list, (_s)->size*sizeof(*(_s)->list));\
    }\
    (_s)->list[(_s)->top++] = _v;\
  } while(0)

#define nstack_pop(_s) (_s)->top--
#define nstack_top(_s, _d) ((_s)->top > 0 ? (_s)->list[(_s)->top-1] : (_d))
#define nstack_free(_s) free((_s)->list)


/* Malloc wrappers that exit on OOM */
void *xmalloc(size_t);
void *xcalloc(size_t, size_t);
void *xrealloc(void *, size_t);

char *xstrdup(const char *);

#endif

