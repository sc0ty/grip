#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdexcept>
#include <map>
#include <string>
#include <sstream>
#include <typeinfo>


class Error : public std::runtime_error
{
	public:
		Error(const char *msg);
		Error(const char *msg, int err);
		virtual ~Error() throw();

		Error &add(const std::string &name, const std::string &value) throw();

		template <typename V>
		Error &add(const std::string &name, const V &value) throw()
		{
			std::stringstream ss;
			ss << value;
			return add(name, ss.str());
		}

		Error &addSource(const char *file, unsigned line,
				const char *func, const char *clas = NULL) throw()
		{
			std::stringstream ss;
			ss << file << ':' << line << "  ";
			if (clas)
				ss << clas << "::";

			ss << func;
			return add("source", ss.str());
		}

		const std::string &get(const std::string &tag) const throw();

		Error &addErrno(int err) throw();

		typedef std::map<std::string, std::string> Tags;
		Tags tags;
};

template <typename T>
class ErrorObj : public Error
{
	public:
		ErrorObj(T &obj, const char *msg) : Error(msg), obj(obj) {}
		ErrorObj(T &obj, const char *msg, int err) : Error(msg, err), obj(obj) {}
		virtual ~ErrorObj() throw() {}

		template <typename V>
		ErrorObj<T> &add(const std::string &name, const V &value) throw()
		{
			Error::add(name, value);
			return *this;
		}

		T &obj;
};

template <typename T>
ErrorObj<T> errorObj(T &obj, const char *msg)
{
	return ErrorObj<T>(obj, msg);
}

template <typename T>
ErrorObj<T> errorObj(T &obj, const char *msg, int err)
{
	return ErrorObj<T>(obj, msg, err);
}


#define ThisError(...) \
	errorObj(*this, __VA_ARGS__) \
	.addSource(__FILE__, __LINE__, __func__, typeid(this).name())

#define FuncError(...) \
	Error(__VA_ARGS__) \
	.addSource(__FILE__, __LINE__, __func__)

#endif
