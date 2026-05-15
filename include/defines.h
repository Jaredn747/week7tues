#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <stdint.h>

#include "string.h"
#include "vid.h"

extern int kstrcpy(char* dest, char* src);

#define printf kprintf
#define bcopy kbcopy
#define memcpy kmemcpy
#define strcpy kstrcpy
#define strlen kstrlen

#define RED 0
#define BLUE 1
#define GREEN 2
#define WHITE 3

typedef uint32_t u32;
typedef uint8_t u8;

#endif