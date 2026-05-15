#ifndef __STRING_H__
#define __STRING_H__

extern int kbcopy(const void *src, void *dest, unsigned int n);
extern char *kmemcpy(void *dest, const void *src, unsigned int n);
extern int kstrlen(char *s);
extern int kstrcmp(char *s1, char *s2);
extern int kstrcpy(char *dest, char *src);

#endif