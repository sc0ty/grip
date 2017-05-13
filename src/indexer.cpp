#include "indexer.h"
#include "index.h"
#include "dir.h"
#include "config.h"
#include "error.h"
#include <cstring>

using namespace std;


static const size_t initBufferSize = 32 * 1024;

#define TRIGRAMS_NO 0x1000000


void sortDatabase(File &oldIdxFile, File &oldDataFile, File &newIdxFile,
		File &newDataFile);


Indexer::Indexer(size_t bufferSize)
	: m_size(0), m_filesTotalSize(0), m_chunksSize(0), m_fileId(0)
{
	if (bufferSize < initBufferSize)
		bufferSize = initBufferSize;

	m_buffer.resize(bufferSize);
	m_ids = new CompressedIds*[TRIGRAMS_NO];
	memset(m_ids, 0, TRIGRAMS_NO*sizeof(CompressedIds*));
}

void Indexer::open(const string &dir)
{
	m_dir = dir;
	makeDirectory(dir + PATH_DELIMITER + GRIP_DIR);
	m_idxFile.open(dir + PATH_DELIMITER + TRIGRAMS_LIST_PATH_TMP, "w+b");
	m_dataFile.open(dir + PATH_DELIMITER + TRIGRAMS_DATA_PATH_TMP, "w+b");
	m_filesFile.open(dir + PATH_DELIMITER + FILE_LIST_PATH_TMP, "wb");
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

	freeIds();
	delete [] m_ids;
}

bool Indexer::indexFile(const string &fname)
{
	File file(fname, "rb");
	size_t fileSize = file.size();

	if (fileSize < 3)
		return false;

	uint8_t *data = m_buffer.data();

	// first check only small fragment of file
	size_t size = file.readN(data, 1, initBufferSize, false);
	if (memchr(data, 0, size))
		return false;

	// fill the buffer and check the remaining fragment
	if (!file.eof())
	{
		size_t read = file.readN(data+size, 1, m_buffer.size()-size, false);
		if (memchr(data+size, 0, read))
			return false;

		size += read;
	}

	uint32_t fileId = addFile(fname);
	uint32_t trigram = '\n';

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

			// don't keep trigrams with newline symbol in the middle as we are
			// grepping whole lines only
			if (data[i] == '\n')
				trigram = '\n';
		}

		if (file.eof())
			break;

		size = file.readN(data, 1, m_buffer.size());
	}

	if (trigram & 0xff00)
	{
		// add newline at the end of file to ensure '$' works properly
		trigram <<= 8;
		trigram |= '\n';
		addTrigram(trigram, fileId);
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
	CompressedIds **ids = m_ids + (trigram & 0x00ffffff);
	if (*ids == NULL)
		*ids = new CompressedIds();

	m_size += (*ids)->add(fileId);
}

void Indexer::freeIds()
{
	for (uint32_t trigram = 0; trigram < TRIGRAMS_NO; trigram++)
	{
		if (m_ids[trigram] != NULL)
		{
			delete m_ids[trigram];
			m_ids[trigram] = NULL;
		}
	}
}

void Indexer::write()
{
	m_filesFile.write(m_fileList.c_str(), m_fileList.size()*sizeof(char));
	m_filesFile.flush();

	Index index;
	index.offset = m_dataFile.tell();

	for (uint32_t trigram = 0; trigram < TRIGRAMS_NO; trigram++)
	{
		CompressedIds *ids = m_ids[trigram];
		if (ids == NULL || ids->empty())
			continue;

		index.trigram = trigram;
		index.lastId = ids->lastId();
		index.size = ids->size();

		m_idxFile.writeObj(index);
		m_dataFile.write(ids->getData(), index.size);

		ids->clearChunk();
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
	freeIds();

	File newIdxFile(TRIGRAMS_LIST_PATH, "wb");
	File newDataFile(TRIGRAMS_DATA_PATH, "wb");

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
