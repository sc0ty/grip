#include "print.h"
#include <cstdarg>
#include <cassert>


using namespace std;


static int g_cursorPos = 0;
static FILE *g_stream = stdout;


#ifndef NDEBUG
static void checkEndLine(const char *c)
{
	while (*c)
	{
		assert(*c != '\n');
		assert(*c != '\r');
		assert(*c != '\v');
		c++;
	}
}
#define endLineAssert(s)  checkEndLine(s)

#else
#define endLineAssert(s)
#endif


void setPrintStream(FILE *stream)
{
	g_stream = stream;
}

void print(const char *fmt, ...)
{
	endLineAssert(fmt);
	va_list args;
	va_start(args, fmt);
	g_cursorPos += vfprintf(g_stream, fmt, args);
	va_end(args);
	fflush(g_stream);
}

void println(const char *fmt, ...)
{
	endLineAssert(fmt);
	va_list args;
	va_start(args, fmt);
	if (g_cursorPos)
	{
		g_cursorPos = 0;
		fprintf(g_stream, "\n");
	}
	vfprintf(g_stream, fmt, args);
	fprintf(g_stream, "\n");
	va_end(args);
}

void printnl()
{
	fprintf(g_stream, "\n");
	g_cursorPos = 0;
}

void reprint(const char *fmt, ...)
{
	endLineAssert(fmt);
	va_list args;
	va_start(args, fmt);
	fprintf(g_stream, "\r");
	int newpos = vfprintf(g_stream, fmt, args) - 1;
	if (newpos < g_cursorPos)
		fprintf(g_stream, "%*c", g_cursorPos-newpos, ' ');
	g_cursorPos = newpos;
	fflush(g_stream);
	va_end(args);
}


string humanReadableSize(size_t bytes)
{
	char buffer[64];
	float b = bytes;

	if (bytes > 1024*1024*1024)
		snprintf(buffer, sizeof(buffer), "%.1f GB", b/(1024.f*1024.f*1024.f));
	else if (bytes > 1024*1024)
		snprintf(buffer, sizeof(buffer), "%.1f MB", b/(1024.f*1024.f));
	else if (bytes > 1024)
		snprintf(buffer, sizeof(buffer), "%.1f kB", b/(1024.f));
	else
		snprintf(buffer, sizeof(buffer), "%u B", (unsigned)bytes);

	return buffer;
}
