#include "grep.h"
#include "pattern.h"
#include "file.h"
#include "fileline.h"
#include <cstdio>
#include <cstring>
#include <list>

using namespace std;


#define COL_START(col) "\33[" col "m\33[K"
#define COL_END        "\33[m\33[K"

/* colors from grep command */
#define COL_MATCH	COL_START("01;31")	// bold red
#define COL_FILE	COL_START("35")		// magenta
#define COL_LINE	COL_START("32")		// green
#define COL_SEP		COL_START("36")		// cyan

#define IS_WORD_CHAR(x)	( \
	((x)>='a' && (x)<='z') || \
	((x)>='A' && (x)<='Z') || \
	((x)>='0' && (x)<='9') || \
	((x)=='_') )


Grep::Grep() :
	m_matchMode(MATCH_DEFAULT),
	m_colorOutput(false),
	m_beforeContext(0),
	m_afterContext(0),
	m_lastLine(0)
{}

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
	Pattern::Match match = firstMatch;
	char sep = match.pos ? ':' : '-';

	if (m_beforeContext || m_afterContext)
	{
		if (m_lastLine && (m_lastLine+1 != lineNo))
			puts(m_colorOutput ? COL_SEP "--" COL_END : "--");

		m_lastLine = lineNo;
	}

	if (m_colorOutput)
	{
		printf(COL_FILE "%s" COL_SEP "%c" COL_LINE "%u" COL_SEP "%c" COL_END,
				fname, sep, lineNo, sep);

		while (match.pos)
		{
			printf("%.*s" COL_MATCH "%.*s" COL_END,
					(int)(match.pos-line), line,
					(int)match.len, match.pos);

			line = match.pos + match.len;
			match = matchStr(line, false);
		}

		printf("%s\n", line);
	}
	else
	{
		printf("%s%c%u%c%s\n", fname, sep, lineNo, sep, line);
	}
}
