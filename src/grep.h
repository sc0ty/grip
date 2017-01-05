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

		void outputFormat(bool color);

		void setBeforeContext(unsigned linesNo);
		void setAfterContext(unsigned linesNo);

		bool grepFile(const std::string &fname);

	private:
		Pattern::Match matchStr(const char *str, bool wholeLine) const;

		void printMatch(const char *fname, unsigned lineNo, const char *line,
				const Pattern::Match &firstMatch = Pattern::Match());

	private:
		std::vector<Pattern*> m_patterns;
		MatchMode m_matchMode;
		bool m_colorOutput;

		unsigned m_beforeContext;
		unsigned m_afterContext;
		unsigned m_lastLine;
};

#endif

