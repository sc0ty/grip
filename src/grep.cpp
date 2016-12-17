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


static string &toLower(string &str);


Grep::Grep() :
	m_caseSensitive(true),
	m_wholeWordMatch(false),
	m_colorOutput(false),
	m_beforeContext(0),
	m_afterContext(0),
	m_lastLine(0)
{}

void Grep::addPattern(const string &pattern)
{
	if (m_caseSensitive)
	{
		m_patterns.push_back(pattern);
	}
	else
	{
		string p = pattern;
		m_patterns.push_back(toLower(p));
	}
}

void Grep::caseInsensitive()
{
	m_caseSensitive = false;
	for (string &pattern : m_patterns)
		toLower(pattern);
}

void Grep::wholeWordMatch(bool wholeWord)
{
	m_wholeWordMatch = wholeWord;
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
		Match match = matchStr(line);

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

const char *Grep::matchStr(const char *str, const string &matchPattern) const
{
	const char *res = NULL;
	const char *pattern = matchPattern.c_str();

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

		if (IS_WORD_CHAR(res[matchPattern.size()]))
			return NULL;
	}

	return res;
}

Grep::Match Grep::matchStr(const char *str) const
{
	Match firstMatch;

	for (const string &pattern : m_patterns)
	{
		const char *match = matchStr(str, pattern);
		if ((match) && (firstMatch.pos == NULL || match < firstMatch.pos))
		{
			firstMatch.pos = match;
			firstMatch.len = pattern.size();
		}
	}

	return firstMatch;
}

void Grep::printMatch(const char *fname, unsigned lineNo, const char *line,
		const Match &firstMatch)
{
	Match match = firstMatch;
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
			match = matchStr(line);
		}

		printf("%s\n", line);
	}
	else
	{
		printf("%s%c%u%c%s\n", fname, sep, lineNo, sep, line);
	}
}

Grep::Match::Match() : pos(NULL), len(0)
{}

string &toLower(string &str)
{
	for (size_t i = 0; i < str.length(); i++)
		str[i] = TO_LOWER(str[i]);

	return str;
}
