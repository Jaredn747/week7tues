/******** vid.c file : LCD driver ********
00 timing0
04 timing1
08 timing2
0C timing3
10 upperPanelframeBaseAddressRegister // use the upper panel
14 lowerPanelFrameBaseAddressRegister // NOT use lower panel
18 controlRegister // NOTE: QEMU emulated PL110 CR is at 0x18
********************************************************/
#include <stdint.h>

#include "defines.h"
#include "vid.h"

extern char image0_start;
int color;
uint8_t cursor;
int volatile *fb;
int row, col, scroll_row;
unsigned char *font;
int WIDTH = 640; // scan line width, default to 640
int HEIGHT = 480;
char *tab = "0123456789ABCDEF";

int fbuf_init() {
    int i;
    fb = (int *)0x200000; // frame buffer at 2MB-4MB
    font = _binary_font_start; // font bitmap
    /********* for 640x480 VGA mode *******************/
    *(volatile unsigned int *)(0x1000001c) = 0x2C77;
    *(volatile unsigned int *)(0x10120000) = 0x3F1F3F9C;
    *(volatile unsigned int *)(0x10120004) = 0x090B61DF;
    *(volatile unsigned int *)(0x10120008) = 0x067F1800;
    *(volatile unsigned int *)(0x10120010) = 0x200000; // at 2MB
    *(volatile unsigned int *)(0x10120018) = 0x82B;
    /********** for 800X600 SVGA mode ******************
    *(volatile unsigned int *)(0x1000001c) = 0x2CAC; // 800x600
    *(volatile unsigned int *)(0x10120000) = 0x1313A4C4;
    *(volatile unsigned int *)(0x10120004) = 0x0505F6F7;
    *(volatile unsigned int *)(0x10120008) = 0x071F1800;
    *(volatile unsigned int *)(0x10120010) = 0x200000;
    *(volatile unsigned int *)(0x10120018) = 0x82B;
    **********/
    cursor = 219; // cursor = row 127 in font bitmap
    row = 13;
}

int setpos(int r, int c) {
    row = r;
    col = c;
}

int clrpix(int x, int y) { // clear pixel at (x,y) 
    int pix = y*WIDTH + x;
    fb[pix] = 0x00000000;
}

int setpix(int x, int y) { // set pixel at (x,y)
    int pix = y*WIDTH + x;
    if (color==RED)
        fb[pix] = 0x000000FF;
    if (color==BLUE)
        fb[pix] = 0x00FF0000;
    if (color==GREEN)
    fb[pix] = 0x0000FF00;
    if (color==WHITE) 
        fb[pix] = 0x00FFFFFF;
    if (color==CYAN) 
        fb[pix] = 0x0000FFFF;

}

int dchar(unsigned char c, int x, int y) { // display char at (x,y)
    int r, bit;
    unsigned char *caddress, byte;
    caddress = font + c*16;
    for (r=0; r<16; r++) {
        byte = *(caddress + r);
        for (bit=0; bit<8; bit++){
            if (byte & (1<<(7-bit)))
                setpix(x+bit, y+r);
        }
    }
}

int undchar(unsigned char c, int x, int y) { // erase char at (x,y)
    int row, bit;
    unsigned char *caddress, byte;
    caddress = font + c*16;
    for (row=0; row<16; row++){
        byte = *(caddress + row);
        for (bit=0; bit<8; bit++){
            if (byte & (1<<(7-bit)))
                clrpix(x+bit, y+row);
        }
    }
}

int scroll() { // scrow UP one line (the hard way)
    int i;
    for (i=208*WIDTH; i<WIDTH*HEIGHT; i++){
        fb[i] = fb[i + WIDTH*16];
    }
}

int kpchar(char c, int ro, int co) { // print char at (row, col)
    int x, y;
    x = co*8;
    y = ro*16;
    dchar(c, x, y);
}

int unkpchar(char c, int ro, int co) { // erase char at (row, col)
    int x, y;
    x = co*8;
    y = ro*16;
    undchar(c, x, y);
}

int erasechar() {// erase char at (row,col)
    int r, bit, x, y;
    unsigned char *caddress, byte;
    x = col*8;
    y = row*16;
    for (r=0; r<16; r++){
        for (bit=0; bit<8; bit++){
            clrpix(x+bit, y+r);
        }
    }
}

int clrcursor() { // clear cursor at (row, col)
    unkpchar(cursor, row, col);
}

int putcursor(unsigned char c) { // set cursor at (row, col)
    kpchar(c, row, col);
}

int kputc(char c) { // print char at cursor position
    clrcursor();
    if (c=='\r'){ // return key
        col=0;
        putcursor(cursor);
        return 0;
    }
    if (c=='\n'){ // new line key
        row++;
        if (row>=25){
            row = 24;
            scroll();
        }
        putcursor(cursor);
        return 0;
    }
    if (c=='\b'){ // backspace key
        if (col>0){
            col--;
            erasechar();
            putcursor(cursor);
        }
        return 0;
    }
    // c is ordinary char case
    kpchar(c, row, col);
    col++;
    if (col>=80){
        col = 0;
        row++;
        if (row >= 25){
            row = 24;
            scroll();
        }
    }
    putcursor(cursor);
}

// The following implements kprintf() for formatted printing
int kprints(char *s) {
    while(*s){
        kputc(*s);
        s++;
    }
}

int krpx(int x) {
    char c;
    if (x){
        c = tab[x % 16];
        krpx(x / 16);
        kputc(c);
    } 
}

int kprintx(int x) {
    if (x==0)
        kputc('0');
    else
        krpx(x);
}

int krpu(int x) {
    char c;
    if (x){
        c = tab[x % 10];
        krpu(x / 10);
        kputc(c);
    }
}

int kprintu(int x) {
    if (x==0)
        kputc('0');
    else
        krpu(x);
}

int kprinti(int x) {
    if (x<0){
        kputc('-');
        x = -x;
    }
    kprintu(x);
}

int kprintf(char *fmt,...) {
    int *ip;
    char *cp;
    cp = fmt;
    ip = (int *)&fmt + 1;
    while(*cp){
        if (*cp != '%'){
            kputc(*cp);
            if (*cp=='\n')
                kputc('\r');
            cp++;
            continue;
        }
        cp++;
        switch(*cp){
            case 'c': kputc((char)*ip); break;
            case 's': kprints((char *)*ip); break;
            case 'd': kprinti(*ip); break;
            case 'u': kprintu(*ip); break;
            case 'x': kprintx(*ip); break;
        }
        cp++; ip++;
    }
}

int show_bmp(char *p, int start_row, int start_col){// SAME as before
    int h, w, pixel, rsize, i, j;
    unsigned char r, g, b;
    char *pp;
    int *q = (int *)(p+14); // skip over 14-byte file header
    w = *(q+1); // image width in pixels
    h = *(q+2); // image height in pixels
    p += 54; // p-> pixels in image
    //BMP images are upside down, each row is a multiple of 4 bytes
    rsize = 4*(w + 0); // multiple of 4
    p += (h-1)*rsize; // last row of pixels
    for (i=start_row; i<start_row + h; i++){
        pp = p;
        for (j=start_col; j<start_col + w; j++){
            b = *pp; g = *(pp+1); r = *(pp+2); // BRG values
            pixel = (b<<16) | (g<<8) | r; // pixel value
            fb[i*WIDTH + j] = pixel; // write to frame buffer
            pp += 4; // advance pp to next pixel
        }
        p -= rsize; // to preceding row
    }
    printf("\nBMP image height=%d width=%d\n", h, w);
}