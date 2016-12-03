#ifndef __IO_H__
#define __IO_H__

#include <vector>
#include <string>
#include <cstddef>
#include "error.h"


void readFile(std::vector<uint8_t> &res, const char *fname,
		size_t sizeLimit = 1024*1024);


class File
{
	public:
		static void remove(const std::string &fname,
				bool ignoreNonExisting = false);

		static void rename(const std::string &name, const std::string &newName);

		File();
		File(const std::string &fname, const char *mode);
		File(FILE *fp);
		virtual ~File();

		void open(const std::string &fname, const char *mode);
		void open(FILE *fp);
		void close();
		bool isOpen() const;

		void remove();
		void renameAndClose(const std::string &newName);

		const std::string &getFileName() const;

		bool read(void *data, size_t len, bool throwOnEof = true);
		void write(const void *data, size_t len);

		size_t readN(void *data, size_t len, size_t no, bool throwOnEof = true);
		size_t writeN(const void *data, size_t len, size_t no);

		void writeLine(const std::string &line);

		template <typename T>
		bool readObj(T &data, bool throwOnEof = true)
		{
			return read(&data, sizeof(data), throwOnEof);
		}

		template <typename T>
		void writeObj(const T &data)
		{
			write(&data, sizeof(data));
		}

		template <typename T>
		bool readVector(std::vector<T> &vec, size_t len, bool throwOnEof = true)
		{
			vec.resize(len);
			return read(vec.data(), len * sizeof(T), throwOnEof);
		}

		template <typename T>
		bool readVector(std::vector<T> &vec, bool throwOnEof = true)
		{
			return readVector(vec, size() / sizeof(T), throwOnEof);
		}

		template <typename T>
		void writeVector(const std::vector<T> &vec)
		{
			write(vec.data(), vec.size() * sizeof(T));
		}


		void seek(size_t pos);
		void seekToEnd();
		size_t tell() const;
		size_t size() const;
		bool eof() const;
		void flush();

	protected:
		FILE *m_fp;
		std::string m_fname;
};


class EndOfFile : public Error
{
	public:
		EndOfFile(File &file);
		virtual ~EndOfFile();

		File &file;
};


#endif
