#include "grep.h"
#include "file.h"
#include "fileline.h"
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

#define TO_LOWER(x)		((x)>='A' && (x)<='Z' ? (x)+'a'-'A' : (x))

#define IS_WORD_CHAR(x)	( \
	((x)>='a' && (x)<='z') || \
	((x)>='A' && (x)<='Z') || \
	((x)>='0' && (x)<='9') || \
	((x)=='_') )


Grep::Grep() :
	m_caseSensitive(true),
	m_wholeWordMatch(false),
	m_colorOutput(false),
	m_beforeContext(0),
	m_afterContext(0),
	m_lastLine(0)
{}

void Grep::setPattern(const string &pattern)
{
	m_pattern = pattern;
	if (!m_caseSensitive)
		patternToLower();
}

void Grep::caseInsensitive()
{
	m_caseSensitive = false;
	patternToLower();
}

void Grep::wholeWordMatch(bool wholeWord)
{
	m_wholeWordMatch = wholeWord;
}

void Grep::outputFormat(bool color)
{
	m_colorOutput = color;
}

void Grep::patternToLower()
{
	for (size_t i = 0; i < m_pattern.size(); i++)
		m_pattern[i] = TO_LOWER(m_pattern[i]);
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
	if (m_pattern.empty())
		throw ThisError("pattern not set");

	FileLineReader file(fname);
	list<string> cachedLines;

	unsigned lineNo = 1;
	unsigned lastMatch = 0;
	const char *line, *match;

	while ((line = file.readLine(false)) != NULL)
	{
		match = matchStr(line);

		if (match)
		{
			unsigned no = lineNo - cachedLines.size();
			for (const string &line : cachedLines)
				printMatch(fname.c_str(), no++, line.c_str(), NULL);
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

const char *Grep::matchStr(const char *str) const
{
	const char *res = NULL;
	const char *pattern = m_pattern.c_str();

	if (m_caseSensitive)
	{
		res = strstr(str, pattern);
	}
	else
	{
		const char *p = pattern;
		for (const char *s = str; *s != '\0'; s++)
		{
			if (TO_LOWER(*s) == *p)
			{
				if (*++p == '\0')
				{
					res = s - (p - pattern - 1);
					break;
				}
			}
			else
			{
				p = pattern;
			}
		}
	}

	if (res && m_wholeWordMatch)
	{
		if ((res > str) && IS_WORD_CHAR(res[-1]))
			return NULL;

		if (IS_WORD_CHAR(res[m_pattern.size()]))
			return NULL;
	}

	return res;
}

void Grep::printMatch(const char *fname, unsigned lineNo, const char *line,
		const char *match)
{
	char sep = match ? ':' : '-';

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

		if (match)
		{
			while (match)
			{
				printf("%.*s" COL_MATCH "%.*s" COL_END,
						(int)(match-line), line,
						(int)m_pattern.size(), match);

				line = match + m_pattern.size();
				match = matchStr(line);
			}
		}

		printf("%s\n", line);
	}
	else
	{
		printf("%s%c%u%c%s\n", fname, sep, lineNo, sep, line);
	}
}
