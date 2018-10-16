#include "grep.h"
#include "pattern.h"
#include "file.h"
#include "fileline.h"
#include "print.h"
#include <cstdio>
#include <cstring>
#include <list>

#define COL_MATCH	color::BRed
#define COL_FILE	color::Magenta
#define COL_LINE	color::Green
#define COL_SEP		color::Cyan


using namespace std;


Grep::Grep() :
	m_matchMode(MATCH_DEFAULT),
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
		Pattern::Match match = matchStr(line);

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

Pattern::Match Grep::matchStr(const char *str) const
{
	Pattern::Match firstMatch;

	for (Pattern *pattern : m_patterns)
	{
		Pattern::Match match;

		if (m_matchMode == MATCH_DEFAULT)
			match = pattern->match(str);
		else if (m_matchMode == MATCH_WHOLE_WORD)
			match = pattern->matchWord(str);
		else if (m_matchMode == MATCH_WHOLE_LINE)
			match = pattern->matchAll(str);

		if (match.pos && (firstMatch.pos == NULL || match.pos < firstMatch.pos))
			firstMatch = match;
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

		// workaround for infinite loop in zero-length match
		if (match.len == 0)
			break;

		if (m_matchMode == MATCH_WHOLE_LINE)
			break;

		match = matchStr(line);
	}

	printf("%s\n", line);
}

void Grep::printSepLine()
{

	color::set(COL_SEP);
	puts("--");
	color::reset();
}

void Grep::printFileLine(const char *fname, unsigned lineNo, char sep)
{
	color::set(COL_FILE);
	printf("%s", fname);
	color::set(COL_SEP);
	putchar(sep);
	color::set(COL_LINE);
	printf("%u", lineNo);
	color::set(COL_SEP);
	putchar(sep);
	color::reset();
}

void Grep::printMatch(const Pattern::Match &match)
{
	color::set(COL_MATCH);
	printf("%.*s", (int)match.len, match.pos);
	color::reset();
}

