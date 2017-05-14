#include "dir.h"
#include "case.h"
#include "config.h"
#include "error.h"
#include <string>

using namespace std;


bool isInDirectory(const string &dir, const string &path)
{
	if (dir == path)
		return true;

	if (dir.size() >= path.size())
		return false;

#if (defined(_WIN32) || defined(__WIN32__))
	// case-insensitive comparison under Windows
	for (size_t i = 0; i < dir.size(); i++)
	{
		if (TO_LOWER(dir[i]) != TO_LOWER(path[i]))
			return false;
	}
#else
	if (path.compare(0, dir.size(), dir) != 0)
		return false;
#endif

	if (!dir.empty() && dir[dir.size() - 1] != PATH_DELIMITER)
		return path.size() > dir.size() && path[dir.size()] == PATH_DELIMITER;

	return true;
}

const char *getRelativePath(const string &dir, string &path)
{
	if (dir.size() > path.size())
	{
		throw FuncError("path is not inside directory")
			.add("path", path)
			.add("dir", dir);
	}
	else if (dir.size() < path.size())
		return path.c_str() + dir.size() + 1;
	else
		return path.c_str() + dir.size();
}

string &canonizePath(const char *path, string &result)
{
	const char *p = path;

	if (*p == PATH_DELIMITER)
		result = *p++;
	else
		result.clear();

	while (*p)
	{
		// multiple '/'
		if (p[0] == PATH_DELIMITER)
		{
			p++;
		}

		// current directory '.'
		else if ((p[0] == '.') && IS_PATH_DELIMITER(p[1]))
		{
			p++;
		}

		// parent directory '..'
		else if ((p[0] == '.') && (p[1] == '.') && IS_PATH_DELIMITER(p[2]))
		{
			size_t pos = result.rfind(PATH_DELIMITER);
			if (pos == string::npos)
				result.clear();
			else if (pos == 0)
				result = PATH_DELIMITER;
			else
				result = result.substr(0, pos);

			p += 2;
		}

		// real directory
		else
		{
			if (!result.empty() && (result != "/"))
				result += PATH_DELIMITER;

			for (; !IS_PATH_DELIMITER(*p); p++)
				result += *p;
		}
	}

	return result;
}

string getIndexPath(const string &refdir)
{
	string dir = !refdir.empty() ? refdir : getCurrentDirectory();
	string gdir = dir + PATH_DELIMITER + GRIP_DIR;

	while (!directoryExists(gdir.c_str()))
	{
		size_t pos = dir.rfind(PATH_DELIMITER, dir.size());
		if ((pos == string::npos) || (pos == 0))
			throw FuncError("can't find index database");

		dir = dir.substr(0, pos);
		gdir = dir + PATH_DELIMITER + GRIP_DIR;
	}

	return dir;
}
