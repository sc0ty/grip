#include "fileline.h"

using namespace std;



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
	File::open(fname, "r");
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

char *FileLineReader::readLine(bool throwOnEof)
{
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

	// trim new line character
	while (read && ((m_line[read-1] == '\n') || (m_line[read-1] == '\r')))
		read--;

	m_line[read] = '\0';
	return m_line;
}
