#ifdef USE_BOOST

#include "dir.h"
#include "error.h"
#include "config.h"
#include <string>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost;


void makeDirectory(const string &path)
{
	try
	{
		filesystem::create_directory(path);
	}
	catch (const filesystem::filesystem_error &ex)
	{
		throw FuncError("cannot create directory")
			.add("message", ex.what())
			.add("path", path);
	}
}

string getCurrentDirectory()
{
	try
	{
		string cwd = filesystem::current_path().string();
		if (!cwd.empty() && cwd.back() == PATH_DELIMITER)
			cwd.pop_back();
		return cwd;
	}
	catch (const filesystem::filesystem_error &ex)
	{
		throw FuncError("cannot obtain current working directory")
			.add("message", ex.what());
	}
}

bool directoryExists(const char *path)
{
	try
	{
		filesystem::path p(path);
		return filesystem::exists(p) && filesystem::is_directory(p);
	}
	catch (const filesystem::filesystem_error &ex)
	{
		throw FuncError("cannot open directory")
			.add("message", ex.what())
			.add("path", path);
	}
}

bool isAbsolutePath(const string &path)
{
	return filesystem::path(path).has_root_directory();
}

#endif
