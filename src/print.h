#ifndef __PRINT_H__
#define __PRINT_H__

#include <cstdio>
#include <string>


void setPrintStream(FILE *stream);

void print(const char *fmt, ...);
void println(const char *fmt, ...);
void reprint(const char *fmt, ...);
void printnl();

std::string humanReadableSize(size_t bytes);

#endif
