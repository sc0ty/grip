#include "dbreader.h"
#include "compressedids.h"
#include "file.h"
#include "dir.h"
#include "case.h"
#include "config.h"
#include <set>
#include <cstdio>
#include <cstring>

using namespace std;


#define TRIGRAM_ENC(s)			(((s)[0] << 16) | ((s)[1] << 8) | (s)[2])
#define TRIGRAM_ENC3(a, b, c)	(((a) << 16) | ((b) << 8) | (c))


inline uint32_t trigramToLower(uint32_t trigram)
{
	uint32_t res = 0;
	res |= TO_LOWER(trigram & 0xff);
	res |= TO_LOWER((trigram >> 8) & 0xff) << 8;
	res |= TO_LOWER((trigram >> 16) & 0xff) << 16;
	return res;
}


DbReader::DbReader(const string &dirDb)
{
	string dir = !dirDb.empty() ? dirDb : getIndexPath();

	m_dataFile.open(dir + PATH_DELIMITER + TRIGRAMS_DATA_PATH, "rb");
	File(dir + PATH_DELIMITER + TRIGRAMS_LIST_PATH, "rb").readVector(m_indexes);
	m_fileList.read(dir + PATH_DELIMITER + FILE_LIST_PATH);
}

const CompressedIds &DbReader::get(uint32_t trigram)
{
	Chunks::const_iterator it = m_chunks.find(trigram);
	if (it != m_chunks.end())
		return it->second;

	CompressedIds &ids = m_chunks[trigram];

	size_t x = 0;
	size_t y = m_indexes.size();
	size_t pos = y / 2;
	Index *data = m_indexes.data();

	while (x + 1 < y)
	{
		uint32_t val = data[pos].trigram;

		if (trigram < val)
		{
			y = pos;
			pos = x + (y - x) / 2;
		}
		else if (trigram > val)
		{
			x = pos;
			pos = x + (y - x) / 2;
		}
		else
		{
			readChunks(data[pos], ids);
			return ids;
		}
	}

	if (data[x].trigram == trigram)
		readChunks(data[x], ids);
	else if (data[y].trigram == trigram)
		readChunks(data[y], ids);

	return ids;
}

const vector<Index> DbReader::getIndexes() const
{
	return m_indexes;
}

void DbReader::clearCache()
{
	m_chunks.clear();
}

void DbReader::readChunks(const Index &index, CompressedIds &ids)
{
	m_dataFile.seek(index.offset);
	uint8_t *data = ids.setData(index.size, index.lastId);
	m_dataFile.read(data, index.size);

	ids.validate();
}

const string &DbReader::getFile(uint32_t id) const
{
	return m_fileList.get(id);
}

uint32_t DbReader::getFilesNo() const
{
	return m_fileList.size();
}

