#ifndef _VID_H_
#define _VID_H_
extern int fbuf_init();
extern int setpos(int r, int c);
extern unsigned char _binary_font_start[];
extern int volatile *fb;
extern char image2_start;
extern char image3_start;
extern int color;
extern int kprintf(char *fmt,...);
extern int kpchar(char c, int ro, int co);
extern int unkpchar(char c, int ro, int co);
extern int show_bmp(char *p, int start_row, int start_col);
extern int WIDTH;
extern int HEIGHT;

#define RED 0
#define BLUE 1
#define GREEN 2
#define WHITE 3
#define CYAN 4
#endif