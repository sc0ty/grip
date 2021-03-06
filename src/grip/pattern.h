#ifndef __PATTERN_H__
#define __PATTERN_H__

#include <string>
#include "node.h"


class Pattern
{
	public:
		enum Mode
		{
			FIXED,
			BASIC,
			EXTENDED,
		};

		struct Match
		{
			const char *pos;
			size_t len;

			Match(const char *pos = NULL, size_t len = 0);
		};

	public:
		virtual ~Pattern();
		virtual void tokenize(Node &tree) const = 0;
		virtual Match match(const char *str) const = 0;

		Match matchWord(const char *str) const;
		Match matchAll(const char *str) const;

		static Pattern *create(const std::string &pattern, Mode mode,
				bool caseSensitive = true);
};
#endif
