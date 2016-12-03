#ifndef __FILELINE_H__
#define __FILELINE_H__

#include "file.h"


class FileLineReader : protected File
{
	public:
		FileLineReader();
		FileLineReader(const std::string &fname);
		FileLineReader(FILE *fp);
		virtual ~FileLineReader();

		void open(const std::string &fname);
		void open(FILE *fp);
		void close();
		bool isOpen() const;

		const std::string &getFileName() const;
		size_t size() const;
		bool eof() const;

		char *readLine(bool throwOnEof = true);

	private:
		char *m_line;
		size_t m_len;
};

#endif
