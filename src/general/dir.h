#ifndef __DIR_H__
#define __DIR_H__

#include <string>


#ifndef PATH_DELIMITER
#if (defined(_WIN32) || defined(__WIN32__))
#define PATH_DELIMITER      '\\'
#define PATH_DELIMITER_S    "\\"
#else
#define PATH_DELIMITER      '/'
#define PATH_DELIMITER_S    "/"
#endif
#endif

#define IS_PATH_DELIMITER(x) ((x) == PATH_DELIMITER || (x) == '\0')


void makeDirectory(const std::string &path);
std::string getCurrentDirectory();
bool directoryExists(const char *path);

bool isAbsolutePath(const std::string &path);

// dir and path must be canonical
bool isInDirectory(const std::string &dir, const std::string &path);

// dir and path must be cannonical, assuming path is inside dir
const char *getRelativePath(const std::string &dir, std::string &path);

std::string &canonizePath(const char *path, std::string &result);

inline std::string &canonizePath(const std::string &path, std::string &result)
{
	return canonizePath(path.c_str(), result);
}

inline std::string &canonizePath(std::string &path)
{
	std::string result;
	canonizePath(path, result);
	return path = result;
}

std::string getIndexPath(const std::string &dir = "");

#endif

