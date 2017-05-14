#ifndef USE_BOOST

#include "dir.h"
#include "error.h"
#include "config.h"
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

#if (defined(_WIN32) || defined(__WIN32__))
#define mkdir(A, B) mkdir(A)
#endif


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

	if (!cwd.empty() && cwd.back() == PATH_DELIMITER)
		cwd.pop_back();

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

bool isAbsolutePath(const string &path)
{
	return !path.empty() && path[0] == PATH_DELIMITER;
}

#endif
