#include "pattern.h"
#include "case.h"
#include "error.h"
#include <cstring>

#include <boost/regex.h>

#define IS_WORD_CHAR(x)	( \
	((x)>='a' && (x)<='z') || \
	((x)>='A' && (x)<='Z') || \
	((x)>='0' && (x)<='9') || \
	((x)=='_') )


using namespace std;


class LiteralPattern : public Pattern
{
	public:
		LiteralPattern(const string &pattern) :
			m_pattern(pattern)
		{}

		virtual ~LiteralPattern()
		{}

		virtual void tokenize(Node &tree) const
		{
			tree.parseFixedString(m_pattern, true);
		}

		virtual Match match(const char *str) const
		{
			Match res(strstr(str, m_pattern.c_str()));

			if (res.pos)
				res.len = m_pattern.size();

			return res;
		}

	private:
		string m_pattern;
};


class LiteralCaseInsPattern : public Pattern
{
	public:
		LiteralCaseInsPattern(const string &pattern) :
			m_pattern(pattern)
		{
			for (size_t i = 0; i < pattern.size(); i++)
				m_pattern[i] = TO_LOWER(m_pattern[i]);
		}

		virtual ~LiteralCaseInsPattern()
		{}

		virtual void tokenize(Node &tree) const
		{
			tree.parseFixedString(m_pattern, false);
		}

		virtual Match match(const char *str) const
		{
			const char *p = m_pattern.c_str();

			for (const char *ch = str; *ch != '\0'; ch++)
			{
				if (TO_LOWER(*ch) == *p)
				{
					if (*++p == '\0')
						return Match(ch-m_pattern.size()+1, m_pattern.size());
				}
				else
				{
					p = m_pattern.c_str();
				}
			}

			return Match();
		}

	private:
		string m_pattern;
};


class RegexPattern : public Pattern
{
	public:
		RegexPattern(const string &pattern, bool extended, bool cs) :
			m_pattern(pattern),
			m_extended(extended),
			m_caseSensitive(cs)
		{
			int flags = 0;
			flags |= extended ? REG_EXTENDED : 0;
			flags |= !cs ? REG_ICASE : 0;

			int res = regcomp(&m_regex, pattern.c_str(), flags);
			if (res != 0)
			{
				throw ThisError("malformed regular expression")
					.add("type", "invalid_query")
					.add("expression", pattern)
					.add("msg", getError(res));
			}
		}

		virtual ~RegexPattern()
		{
			regfree(&m_regex);
		}

		virtual void tokenize(Node &tree) const
		{
			tree.parseRegex(m_pattern, m_extended, m_caseSensitive);
		}

		virtual Match match(const char *str) const
		{
			regmatch_t res;
			int code = regexec(&m_regex, str, 1, &res, 0);

			if (code == 0)
			{
				return Match(str + res.rm_so, res.rm_eo - res.rm_so);
			}
			else if (code == REG_NOMATCH)
			{
				return Match();
			}
			else
			{
				throw ThisError("malformed regular expression")
					.add("type", "invalid_query")
					.add("expression", m_pattern)
					.add("msg", getError(code));
			}
		}

	private:
		string getError(int code) const
		{
			char buf[1024];
			regerror(code, &m_regex, buf, sizeof(buf));
			return buf;
		}

	private:
		string m_pattern;
		regex_t m_regex;
		bool m_extended;
		bool m_caseSensitive;
};


Pattern::~Pattern()
{}

Pattern *Pattern::create(const string &pattern, Mode mode, bool caseSensitive)
{
	if (pattern.find('\0') != string::npos || pattern.find('\n') != string::npos)
		throw FuncError("malformed regular expression")
			.add("type", "invalid_query")
			.add("expression", pattern);

	if (mode == FIXED)
	{
		if (caseSensitive)
			return new LiteralPattern(pattern);
		else
			return new LiteralCaseInsPattern(pattern);
	}
	else
	{
		bool extended = (mode == EXTENDED);
		return new RegexPattern(pattern, extended, caseSensitive);
	}
}

Pattern::Match Pattern::matchWord(const char *str) const
{
	Match res;
	for (size_t i = 0; str[i] != '\0'; i++)
	{
		res = match(str + i);

		if (res.pos == NULL)
			break;

		if ((res.pos == str || !IS_WORD_CHAR(res.pos[-1]))
				&& !IS_WORD_CHAR(res.pos[res.len]))
			break;
	}
	return res;
}

Pattern::Match Pattern::matchAll(const char *str) const
{
	Match res = match(str);
	if (res.pos != str || res.pos[res.len] != '\0')
		res.pos = NULL;

	return res;
}

Pattern::Match::Match(const char *pos, size_t len) : pos(pos), len(len)
{}
