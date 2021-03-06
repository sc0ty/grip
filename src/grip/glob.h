#ifndef __GLOB_H__
#define __GLOB_H__

#include <vector>
#include <string>

class Glob
{
	public:
		Glob();

		void addExcludePattern(const std::string &pattern);
		void addIncludePattern(const std::string &pattern);

		void caseSensitive(bool enable);
		void extendedMatch(bool enable);

		bool compare(const std::string &str) const;

	private:
		std::vector<std::string> m_excludes;
		std::vector<std::string> m_includes;
		int m_flags;
};

#endif
