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
		void wholeWordMatch(bool wholeWord);

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
		bool m_wholeWordMatch;
		bool m_colorOutput;

		unsigned m_beforeContext;
		unsigned m_afterContext;
		unsigned m_lastLine;
};

#endif

