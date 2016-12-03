#include "indexer.h"
#include "index.h"
#include "dir.h"
#include "config.h"
#include "error.h"
#include <cstring>

using namespace std;


static const size_t initBufferSize = 32 * 1024;


void sortDatabase(File &oldIdxFile, File &oldDataFile, File &newIdxFile,
		File &newDataFile);


inline static bool isBinaryData(uint8_t *data, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		if (data[i] == 0)
			return true;
	}

	return false;
}


Indexer::Indexer(size_t bufferSize)
	: m_size(0), m_filesTotalSize(0), m_chunksSize(0), m_fileId(0)
{
	if (bufferSize < initBufferSize)
		bufferSize = initBufferSize;

	m_buffer.resize(bufferSize);
}

void Indexer::open(const string &dir)
{
	m_dir = dir;
	makeDirectory(dir + PATH_DELIMITER + GRIP_DIR);
	m_idxFile.open(dir + PATH_DELIMITER + TRIGRAMS_LIST_PATH_TMP, "w+");
	m_dataFile.open(dir + PATH_DELIMITER + TRIGRAMS_DATA_PATH_TMP, "w+");
	m_filesFile.open(dir + PATH_DELIMITER + FILE_LIST_PATH_TMP, "w");
}

void Indexer::close()
{
	m_idxFile.close();
	m_dataFile.close();
	m_filesFile.close();
}

Indexer::~Indexer()
{
	if (m_size > 0)
		write();
}

bool Indexer::indexFile(const string &fname)
{
	File file(fname, "r");
	size_t fileSize = file.size();

	if (fileSize < 3)
		return false;

	uint8_t *data = m_buffer.data();

	// first check only small fragment of file
	size_t size = file.readN(data, 1, initBufferSize, false);
	if (isBinaryData(data, size))
		return false;

	// fill the buffer and check the remaining fragment
	if (!file.eof())
	{
		size_t read = file.readN(data+size, 1, m_buffer.size()-size, false);
		if (isBinaryData(data+size, read))
			return false;

		size += read;
	}

	uint32_t fileId = addFile(fname);
	uint32_t trigram = 0;

	while (size > 0)
	{
		for (size_t i = 0; i < size; i++)
		{
			// ignoring windows newline encoding
			if (data[i] == '\r')
				continue;

			trigram <<= 8;
			trigram |= data[i];

			if (trigram & 0xff0000)
				addTrigram(trigram, fileId);
		}

		if (file.eof())
			break;

		size = file.readN(data, 1, m_buffer.size());
	}

	m_filesTotalSize += fileSize;
	return true;
}

uint32_t Indexer::addFile(const std::string &fname)
{
	m_fileList += fname + '\n';
	return m_fileId++;
}

void Indexer::addTrigram(uint32_t trigram, uint32_t fileId)
{
	m_size += m_trigrams[trigram & 0x00ffffff].add(fileId);
}

void Indexer::write()
{
	m_filesFile.write(m_fileList.c_str(), m_fileList.size()*sizeof(char));
	m_filesFile.flush();

	Index index;
	index.offset = m_dataFile.tell();

	for (Trigrams::iterator it = m_trigrams.begin();
			it != m_trigrams.end(); ++it)
	{
		CompressedIds &ids = it->second;
		if (ids.empty())
			continue;

		index.trigram = it->first;
		index.lastId = ids.lastId();
		index.size = ids.size();

		m_idxFile.writeObj(index);
		m_dataFile.write(ids.getData(), index.size);

		ids.clearChunk();
		index.offset += index.size;
	}

	m_idxFile.flush();
	m_dataFile.flush();

	m_fileList.clear();
	m_size = 0;
}

void Indexer::sortDatabase()
{
	write();

	m_trigrams.clear();

	File newIdxFile(TRIGRAMS_LIST_PATH, "w");
	File newDataFile(TRIGRAMS_DATA_PATH, "w");

	::sortDatabase(m_idxFile, m_dataFile, newIdxFile, newDataFile);
	m_chunksSize = newDataFile.size();

	m_idxFile.remove();
	m_dataFile.remove();

	File::remove(m_dir + PATH_DELIMITER + FILE_LIST_PATH, true);
	m_filesFile.renameAndClose(m_dir + PATH_DELIMITER + FILE_LIST_PATH);
}

size_t Indexer::size() const
{
	return m_size;
}

size_t Indexer::filesNo() const
{
	return m_fileId;
}

size_t Indexer::filesTotalSize() const
{
	return m_filesTotalSize;
}

size_t Indexer::chunksSize() const
{
	return m_dataFile.isOpen() ? m_dataFile.size() : m_chunksSize;
}
