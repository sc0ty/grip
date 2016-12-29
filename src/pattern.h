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
		virtual Match match(const char *str, bool wholeLine) const = 0;

		static Pattern *create(const std::string &pattern, Mode mode,
				bool caseSensitive = true);
};
#endif
