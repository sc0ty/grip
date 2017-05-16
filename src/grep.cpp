#include "grep.h"
#include "pattern.h"
#include "file.h"
#include "fileline.h"
#include <cstdio>
#include <cstring>
#include <list>

#if (defined(_WIN32) || defined(__WIN32__))

#include <windows.h>
static HANDLE console = NULL;
static WORD oldColors = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

/* colors under Windows */
#define COL_MATCH	(FOREGROUND_RED | FOREGROUND_INTENSITY)
#define COL_FILE	(FOREGROUND_RED | FOREGROUND_BLUE)
#define COL_LINE	(FOREGROUND_GREEN)
#define COL_SEP		(FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COL_END     (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

#else

#define COL_START(col) "\33[" col "m\33[K"
#define COL_END        "\33[m\33[K"

/* ASCII colors from grep command */
#define COL_MATCH	COL_START("01;31")	// bold red
#define COL_FILE	COL_START("35")		// magenta
#define COL_LINE	COL_START("32")		// green
#define COL_SEP		COL_START("36")		// cyan

#endif

#define IS_WORD_CHAR(x)	( \
	((x)>='a' && (x)<='z') || \
	((x)>='A' && (x)<='Z') || \
	((x)>='0' && (x)<='9') || \
	((x)=='_') )

using namespace std;


Grep::Grep() :
	m_matchMode(MATCH_DEFAULT),
	m_colorOutput(false),
	m_beforeContext(0),
	m_afterContext(0),
	m_lastLine(0)
{
#if (defined(_WIN32) || defined(__WIN32__))
	if (console == NULL)
	{
		console = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO screenInfo;
		if (GetConsoleScreenBufferInfo(console, &screenInfo))
			oldColors = screenInfo.wAttributes;
	}
#endif
}

Grep::~Grep()
{
	for (Pattern *pattern : m_patterns)
		delete pattern;
}

void Grep::addPattern(Pattern *pattern)
{
	m_patterns.push_back(pattern);
}

void Grep::matchMode(MatchMode mode)
{
	m_matchMode = mode;
}

void Grep::outputFormat(bool color)
{
	m_colorOutput = color;
}

void Grep::setBeforeContext(unsigned linesNo)
{
	m_beforeContext = linesNo;
}

void Grep::setAfterContext(unsigned linesNo)
{
	m_afterContext = linesNo;
}

bool Grep::grepFile(const string &fname)
{
	if (m_patterns.empty())
		throw ThisError("pattern not set");

	FileLineReader file(fname);
	list<string> cachedLines;

	unsigned lineNo = 1;
	unsigned lastMatch = 0;
	const char *line;

	while ((line = file.readLine(false)) != NULL)
	{
		Pattern::Match match = matchStr(line, true);

		if (match.pos)
		{
			unsigned no = lineNo - cachedLines.size();
			for (const string &line : cachedLines)
				printMatch(fname.c_str(), no++, line.c_str());
			cachedLines.clear();

			printMatch(fname.c_str(), lineNo, line, match);
			lastMatch = lineNo;
		}
		else
		{
			if (lastMatch && (lineNo - lastMatch <= m_afterContext))
			{
				printMatch(fname.c_str(), lineNo, line, match);
			}
			else if (m_beforeContext)
			{
				if (cachedLines.size() >= m_beforeContext)
					cachedLines.pop_front();

				cachedLines.push_back(line);
			}
		}

		lineNo++;
	}

	if (m_lastLine)
		m_lastLine = (unsigned) -1;

	return lastMatch;
}

Pattern::Match Grep::matchStr(const char *str, bool wholeLine) const
{
	Pattern::Match firstMatch;

	for (Pattern *pattern : m_patterns)
	{
		Pattern::Match match = pattern->match(str, wholeLine);
		if (match.pos && (firstMatch.pos == NULL || match.pos < firstMatch.pos))
		{
			if (m_matchMode == MATCH_WHOLE_LINE)
			{
				if (!wholeLine)
					break;

				if ((str != match.pos) || (strlen(str) != match.len))
					break;
			}
			else if (m_matchMode == MATCH_WHOLE_WORD)
			{
				if ((match.pos > str) && IS_WORD_CHAR(match.pos[-1]))
					continue;

				if (IS_WORD_CHAR(match.pos[match.len]))
					continue;
			}

			firstMatch = match;
		}
	}

	return firstMatch;
}

void Grep::printMatch(const char *fname, unsigned lineNo, const char *line,
		const Pattern::Match &firstMatch)
{
	if (m_beforeContext || m_afterContext)
	{
		if (m_lastLine && (m_lastLine+1 != lineNo))
			printSepLine();

		m_lastLine = lineNo;
	}

	Pattern::Match match = firstMatch;
	printFileLine(fname, lineNo, match.pos ? ':' : '-');

	while (match.pos)
	{
		printf("%.*s", (int)(match.pos-line), line);
		printMatch(match);

		line = match.pos + match.len;
		match = matchStr(line, false);
	}

	printf("%s\n", line);
}

#if (defined(_WIN32) || defined(__WIN32__))

void Grep::printSepLine()
{
	if (m_colorOutput)
	{
		SetConsoleTextAttribute(console, COL_SEP);
		puts("--");
		SetConsoleTextAttribute(console, oldColors);
	}
	else
	{
		puts("--");
	}
}

void Grep::printFileLine(const char *fname, unsigned lineNo, char sep)
{
	if (m_colorOutput)
	{
		SetConsoleTextAttribute(console, COL_FILE);
		printf("%s", fname);
		SetConsoleTextAttribute(console, COL_SEP);
		putchar(sep);
		SetConsoleTextAttribute(console, COL_LINE);
		printf("%u", lineNo);
		SetConsoleTextAttribute(console, COL_SEP);
		putchar(sep);
		SetConsoleTextAttribute(console, oldColors);
	}
	else
	{
		printf("%s%c%u%c", fname, sep, lineNo, sep);
	}
}

void Grep::printMatch(const Pattern::Match &match)
{
	if (m_colorOutput)
	{
		SetConsoleTextAttribute(console, COL_MATCH);
		printf("%.*s", (int)match.len, match.pos);
		SetConsoleTextAttribute(console, oldColors);
	}
	else
	{
		printf("%.*s", (int)match.len, match.pos);
	}
}

#else

void Grep::printSepLine()
{
	puts(m_colorOutput ? COL_SEP "--" COL_END : "--");
}

void Grep::printFileLine(const char *fname, unsigned lineNo, char sep)
{
	if (m_colorOutput)
	{
		printf(COL_FILE "%s" COL_SEP "%c" COL_LINE "%u" COL_SEP "%c" COL_END,
				fname, sep, lineNo, sep);
	}
	else
	{
		printf("%s%c%u%c", fname, sep, lineNo, sep);
	}
}

void Grep::printMatch(const Pattern::Match &match)
{
	if (m_colorOutput)
		printf(COL_MATCH "%.*s" COL_END, (int)match.len, match.pos);
	else
		printf("%.*s", (int)match.len, match.pos);
}

#endif
