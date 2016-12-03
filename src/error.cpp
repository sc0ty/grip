#include "error.h"
#include <cstring>

using namespace std;


Error::Error(const char *msg) : runtime_error(msg)
{}

Error::Error(const char *msg, int err) : runtime_error(msg)
{
	addErrno(err);
}

Error::~Error() throw()
{}

Error &Error::add(const string &name, const string &value) throw()
{
	tags[name] = value;
	return *this;
}

const string &Error::get(const string &tag) const throw()
{
	static const string emptyString = "";

	Tags::const_iterator it = tags.find(tag);
	return it != tags.end() ? it->second : emptyString;
}

Error &Error::addErrno(int err) throw()
{
	add("errno", err);
	add("msg", strerror(err));
	return *this;
}

