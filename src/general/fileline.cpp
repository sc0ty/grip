#include "fileline.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

using namespace std;

#ifndef MAX_LINE_SIZE
#define MAX_LINE_SIZE	(16 * 1024)
#endif


FileLineReader::FileLineReader() : m_line(NULL), m_len(0)
{}

FileLineReader::FileLineReader(const string &fname) : m_line(NULL), m_len(0)
{
	open(fname.c_str());
}

FileLineReader::FileLineReader(FILE *fp) : File(fp), m_line(NULL), m_len(0)
{}

FileLineReader::~FileLineReader()
{
	free(m_line);
}

void FileLineReader::open(const string &fname)
{
	File::open(fname, "rb");
}

void FileLineReader::open(FILE *fp)
{
	File::open(fp);
}

void FileLineReader::close()
{
	File::close();
}

bool FileLineReader::isOpen() const
{
	return File::isOpen();
}

const string &FileLineReader::getFileName() const
{
	return File::getFileName();
}

size_t FileLineReader::size() const
{
	return File::size();
}

bool FileLineReader::eof() const
{
	return File::eof();
}

char *FileLineReader::readLine(bool throwOnEof, size_t *length)
{

#if defined _POSIX_C_SOURCE && _POSIX_C_SOURCE >= 200809L || defined _XOPEN_SOURCE && _XOPEN_SOURCE >= 700

	ssize_t read = getline(&m_line, &m_len, m_fp);
	if (read == -1)
	{
		if (feof(m_fp))
		{
			if (throwOnEof)
				throw EndOfFile(*this);
			else
				return NULL;
		}
		else
			throw ThisError("read failed", errno).add("file", getFileName());
	}

#else

	if (m_line == NULL)
	{
		m_len = MAX_LINE_SIZE;
		m_line = (char*) malloc(m_len);
	}

	char *res = fgets(m_line, m_len, m_fp);
	if (res == NULL)
	{
		if (feof(m_fp))
		{
			if (throwOnEof)
				throw EndOfFile(*this);
			else
				return NULL;
		}
		else
			throw ThisError("read failed", errno).add("file", getFileName());
	}

	size_t read  = strlen(m_line);

#endif

	// trim new line character
	while (read && ((m_line[read-1] == '\n') || (m_line[read-1] == '\r')))
		read--;

	m_line[read] = '\0';

	if (length)
		*length = read;
	return m_line;
}

bool FileLineReader::readLine(string &line)
{
	size_t len = 0;
	char *res = readLine(false, &len);
	if (res)
	{
		line = string(res, len);
		return true;
	}
	return false;
}
