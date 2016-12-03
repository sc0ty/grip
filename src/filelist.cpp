#include "filelist.h"
#include "file.h"
#include "fileline.h"

using namespace std;


uint32_t Files::add(const char *fname)
{
	m_files.push_back(fname);
	return m_files.size() - 1;
}

uint32_t Files::getNextId() const
{
	return m_files.size();
}

void Files::clear()
{
	m_files.clear();
}

const string &Files::get(uint32_t id) const
{
	return m_files[id];
}

uint32_t Files::size() const
{
	return m_files.size();
}

bool Files::empty() const
{
	return m_files.empty();
}

void Files::write(const string &fname) const
{
	File file(fname, "w");

	for (auto &name : m_files)
		file.writeLine(name);
}

void Files::read(const string &fname)
{
	clear();
	FileLineReader file(fname);

	const char *line;
	while ((line = file.readLine(false)) != NULL)
	{
		m_files.push_back(line);
	}
}

Files::iterator Files::begin()
{
	return m_files.begin();
}

Files::iterator Files::end()
{
	return m_files.end();
}

Files::const_iterator Files::begin() const
{
	return m_files.begin();
}

Files::const_iterator Files::end() const
{
	return m_files.end();
}

