#include "file.h"
#include "error.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <boost/filesystem.hpp>

using namespace boost;
using namespace std;


EndOfFile::EndOfFile(File &file) :
	Error("end of file"), file(file)
{
	add("file", file.getFileName());
}

EndOfFile::~EndOfFile()
{}


void readFile(vector<uint8_t> &res, const char *fname, size_t sizeLimit)
{
	File file(fname, "rb");
	size_t size = file.size();
	if ((size_t)size > sizeLimit)
		throw FuncError("file size limit exceeded").add("file", fname);

	res.resize(size);
	file.read(res.data(), size);
}

void File::remove(const string &fname, bool ignoreNonExisting)
{
	(void) ignoreNonExisting;
	try
	{
		filesystem::remove(fname);
	}
	catch (const filesystem::filesystem_error &ex)
	{
		throw FuncError("cannot remove file")
			.add("message", ex.what())
			.add("file", fname);
	}
}

void File::rename(const std::string &name, const std::string &newName)
{
	try
	{
		filesystem::rename(name, newName);
	}
	catch (const filesystem::filesystem_error &ex)
	{
		throw FuncError("cannot rename file")
			.add("message", ex.what())
			.add("file", name)
			.add("newFile", newName);
	}
}

File::File() : m_fp(NULL)
{}

File::File(const string &fname, const char *mode) : m_fp(NULL)
{
	open(fname, mode);
}

File::File(FILE *fp) : m_fp(fp)
{}

File::~File()
{
	close();
}

void File::open(const string &fname, const char *mode)
{
	m_fp = fopen(fname.c_str(), mode);
	if (m_fp == NULL)
		throw ThisError("cannot open file", errno).add("file", fname);
	m_fname = fname;
}

void File::open(FILE *fp)
{
	m_fp = fp;
}

void File::close()
{
	if (m_fp && !m_fname.empty())
	{
		fclose(m_fp);
		m_fp = NULL;
		m_fname.clear();
	}
}

void File::renameAndClose(const string &newName)
{
	if (m_fp == NULL)
		throw ThisError("cannot rename file - file not opened")
			.add("newFile", newName);

	if (m_fname.empty())
		throw ThisError("cannot rename file - file name is empty")
			.add("newFile", newName);

	string oldName = getFileName();

	close();
	rename(oldName, newName);
}

void File::remove()
{
	string fname = getFileName();
	close();
	remove(fname);
}

bool File::isOpen() const
{
	return m_fp;
}

const string &File::getFileName() const
{
	return m_fname;
}

size_t File::size() const
{
	if (fseek(m_fp, 0, SEEK_END) != 0)
		throw ThisError("seek failed", errno).add("file", getFileName());

	long size = ftell(m_fp);
	::rewind(m_fp);

	if (size == -1)
		throw ThisError("tell file failed", errno).add("file", getFileName());

	return size;
}

bool File::eof() const
{
	return m_fp && feof(m_fp);
}

void File::flush()
{
	if (fflush(m_fp) != 0)
		throw ThisError("flush failed", errno).add("file", getFileName());
}

bool File::read(void *data, size_t len, bool throwOnEof)
{
	if (len > 0)
	{
		if (fread(data, len, 1, m_fp) != 1)
		{
			if (eof())
			{
				if (throwOnEof)
					throw EndOfFile(*this);
				else
					return false;
			}
			else
				throw ThisError("read failed", errno).add("file", getFileName());
		}
	}
	return true;
}

void File::write(const void *data, size_t len)
{
	if (len > 0)
	{
		if (fwrite(data, len, 1, m_fp) != 1)
			throw ThisError("write failed", errno).add("file", getFileName());
	}
}

size_t File::readN(void *data, size_t len, size_t no, bool throwOnEof)
{
	if ((len == 0) || (no == 0))
		return 0;

	size_t read = fread(data, len, no, m_fp);
	if (read == 0)
	{
		if (eof())
		{
			if (throwOnEof)
				throw EndOfFile(*this);
			else
				return 0;
		}
		else
			throw ThisError("read failed", errno).add("file", getFileName());
	}

	return read;
}

size_t File::writeN(const void *data, size_t len, size_t no)
{
	size_t write = fwrite(data, len, no, m_fp);
	if (write == 0)
		throw ThisError("write failed", errno).add("file", getFileName());

	return write;
}

void File::writeLine(const string &line)
{
	string data = line + '\n';
	write(data.c_str(), data.size() * sizeof(char));
}

void File::seek(size_t pos)
{
	if (fseek(m_fp, pos, SEEK_SET) != 0)
		throw ThisError("seek failed", errno).add("file", getFileName());
}

void File::seekToEnd()
{
	if (fseek(m_fp, 0, SEEK_END) != 0)
		throw ThisError("seek failed", errno).add("file", getFileName());
}

void File::rewind()
{
	::rewind(m_fp);
}

size_t File::tell() const
{
	long size = ftell(m_fp);
	if (size == -1)
		throw ThisError("tell failed", errno).add("file", getFileName());

	return size;
}
