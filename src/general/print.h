#ifndef __PRINT_H__
#define __PRINT_H__

#include <string>


void setPrintStream(FILE *stream);

/* print functions with line re-print functionality */
void print(const char *fmt, ...);
void println(const char *fmt, ...);
void reprint(const char *fmt, ...);
void printnl();

std::string humanReadableSize(size_t bytes);

namespace color
{
	enum Color
	{
		Black, Red, Green, Yellow, Blue, Magenta, Cyan, White,
		BBlack, BRed, BGreen, BYellow, BBlue, BMagenta, BCyan, BWhite
	};

	void init();
	void mode(bool color);
	void set(Color color);
	void reset();
}

#endif
