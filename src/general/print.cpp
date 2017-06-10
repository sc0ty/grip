#include "print.h"
#include <cstdio>
#include <cstdarg>
#include <cassert>

#if defined(_POSIX_C_SOURCE)
#include <unistd.h>
#endif

using namespace std;


static int g_cursorPos = 0;
static FILE *g_stream = stdout;
static bool g_color = false;


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
		fputc('\n', g_stream);
	}
	vfprintf(g_stream, fmt, args);
	fprintf(g_stream, "\n");
	va_end(args);
}

void printnl()
{
	fputc('\n', g_stream);
	g_cursorPos = 0;
}

void reprint(const char *fmt, ...)
{
	endLineAssert(fmt);
	va_list args;
	va_start(args, fmt);
	fputc('\r', g_stream);
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


namespace color
{

	void mode(bool cm)
	{
		g_color = cm;
	}

#if defined(_WIN32) || defined(__WIN32__)

#include <windows.h>
	static HANDLE g_console = NULL;
	static WORD g_oldColors = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

	void init()
	{
		g_color = true;

		if (g_console == NULL)
		{
			g_console = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO screenInfo;
			if (GetConsoleScreenBufferInfo(g_console, &screenInfo))
				g_oldColors = screenInfo.wAttributes;
		}
	}

	void set(Color c)
	{
		if (g_color && g_console)
		{
			unsigned r = (c & 0x0a) | ((c & 0x01) << 2) | ((c & 0x04) >> 2);
			SetConsoleTextAttribute(g_console, r);
		}
	}

	void reset()
	{
		if (g_color && g_console)
		{
			SetConsoleTextAttribute(g_console, g_oldColors);
		}
	}

#else

	void init()
	{
#if defined(_POSIX_C_SOURCE)
		g_color = isatty(STDOUT_FILENO);
#endif
	}

	void set(Color color)
	{
		if (g_color)
		{
			unsigned c = (color & 0x07) + 30;
			if (color & 0x08)
				fprintf(g_stream, "\33[01;%um\33[K", c);
			else
				fprintf(g_stream, "\33[%um\33[K", c);
		}
	}

	void reset()
	{
		if (g_color)
		{
			fputs("\33[m\33[K", g_stream);
		}
	}

#endif
}

