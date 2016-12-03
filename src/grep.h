#ifndef __GREP_H__
#define __GREP_H__

#include <string>
#include <vector>
#include <stdint.h>


class Grep
{
	public:
		Grep();

		void setPattern(const std::string &pattern);
		void caseSensitive(bool sensitive);
		void outputFormat(bool color);
		void setBeforeContext(unsigned linesNo);
		void setAfterContext(unsigned linesNo);

		bool grepFile(const std::string &fname);

	private:
		void patternToLower();
		void printMatch(const char *fname, unsigned lineNo, const char *line,
				const char *match = NULL);

	private:
		std::string m_pattern;
		bool m_caseSensitive;
		bool m_colorOutput;

		unsigned m_beforeContext;
		unsigned m_afterContext;
		unsigned m_lastLine;
};

#endif

