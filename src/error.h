#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdexcept>
#include <map>
#include <string>
#include <sstream>


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


#ifdef NDEBUG

#define ThisError(...) errorObj(*this, __VA_ARGS__)
#define FuncError(...) Error(__VA_ARGS__)

#else

#include <typeinfo>

#define ThisError(...) \
	errorObj(*this, __VA_ARGS__) \
	.add("class", std::string(typeid(this).name())) \
	.add("method", __FUNCTION__) \
	.add("srcfile", __FILE__) \
	.add("srcline", __LINE__)

#define FuncError(...) \
	Error(__VA_ARGS__) \
	.add("function", __func__) \
	.add("srcfile", __FILE__) \
	.add("srcline", __LINE__)

#endif

#endif
