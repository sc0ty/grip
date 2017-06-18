#ifndef __GREP_H__
#define __GREP_H__

#include <string>
#include <vector>
#include <stdint.h>
#include "pattern.h"


class Grep
{
	public:
		Grep();
		~Grep();

		void addPattern(Pattern *pattern);

		enum MatchMode { MATCH_DEFAULT, MATCH_WHOLE_WORD, MATCH_WHOLE_LINE };
		void matchMode(MatchMode mode);

		void setBeforeContext(unsigned linesNo);
		void setAfterContext(unsigned linesNo);

		bool grepFile(const std::string &fname);

	private:
		Pattern::Match matchStr(const char *str) const;

		void printMatch(const char *fname, unsigned lineNo, const char *line,
				const Pattern::Match &firstMatch = Pattern::Match());

		void printSepLine();
		void printFileLine(const char *fname, unsigned lineNo, char sep);
		void printMatch(const Pattern::Match &match);

	private:
		std::vector<Pattern*> m_patterns;
		MatchMode m_matchMode;

		unsigned m_beforeContext;
		unsigned m_afterContext;
		unsigned m_lastLine;
};

#endif

