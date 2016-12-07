#include "glob.h"
#include "dir.h"
#include "error.h"
#include <fnmatch.h>


using namespace std;


void Glob::addExcludePattern(const string &pattern)
{
	m_excludes.push_back(pattern);
}

void Glob::addIncludePattern(const string &pattern)
{
	m_includes.push_back(pattern);
}

bool Glob::compare(const string &str) const
{
	size_t pos = str.rfind(PATH_DELIMITER);
	const char *fname = str.c_str() + (pos == string::npos ? 0 : pos + 1);

	for (const string &pattern : m_excludes)
	{
		if (fnmatch(pattern.c_str(), fname, 0) == 0)
			return false;
	}

	if (m_includes.empty())
		return true;

	for (const string &pattern : m_includes)
	{
		if (fnmatch(pattern.c_str(), fname, 0) == 0)
			return true;
	}

	return false;
}
