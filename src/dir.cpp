#include "dir.h"
#include "error.h"
#include "config.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <cstring>
#include <unistd.h>

using namespace std;


void makeDirectory(const string &path)
{
	if ((mkdir(path.c_str(), 0777) != 0) && (errno  != EEXIST))
		throw FuncError("cannot create directory", errno).add("path", path);
}

string getCurrentDirectory()
{
	size_t size = 1024;
	char *buffer = new char[size];
	const char *res = NULL;

	while (((res = getcwd(buffer, size)) == NULL) && (errno == ERANGE))
	{
		delete [] buffer;
		buffer = new char[size *= 2];
	}

	string cwd(res);

	if (res)
		cwd = buffer;

	delete [] buffer;

	if (res == NULL)
		throw FuncError("cannot obtain current working directory", errno);

	return cwd;
}

bool directoryExists(const char *path)
{
	DIR *dir = opendir(path);
	if (dir == NULL)
	{
		if ((errno == ENOENT) || (errno == ENOTDIR))
			return false;
		else
			throw FuncError("cannot open directory", errno).add("path", path);
	}

	closedir(dir);
	return true;
}

bool isInDirectory(const string &dir, const string &path)
{
	if (dir == path)
		return true;

	return (dir.size() < path.size()) && (path.compare(0, dir.size(), dir) == 0);
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


#define IS_PATH_DELIMITER(x)	((x) == PATH_DELIMITER || (x) == '\0')

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
