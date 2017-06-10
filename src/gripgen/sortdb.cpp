#include "indexer.h"
#include "index.h"
#include "file.h"
#include <map>
#include <vector>

using namespace std;


typedef vector<Index> Chunks;
typedef map<uint32_t /* trigram */, Chunks> Trigrams;


static void readTrigrams(File &idxFile, Trigrams &trigrams);
static size_t getLongestTrigramSize(const Trigrams &trigrams);
static size_t readChunksData(const Chunks &chunks, File &dataFile,
		uint8_t *result, uint32_t &lastId);


void sortDatabase(File &oldIdxFile, File &oldDataFile, File &newIdxFile,
		File &newDataFile)
{
	Trigrams trigrams;
	readTrigrams(oldIdxFile, trigrams);

	vector<uint8_t> buffer;
	buffer.resize(getLongestTrigramSize(trigrams));
	uint8_t *data = buffer.data();

	Index index;
	index.offset = newDataFile.tell();

	for (Trigrams::const_iterator it = trigrams.begin();
			it != trigrams.end(); ++it)
	{
		const Chunks &chunks = it->second;
		index.trigram = it->first;
		index.size = readChunksData(chunks, oldDataFile, data, index.lastId);

		newDataFile.write(data, index.size);
		newIdxFile.writeObj(index);
		index.offset += index.size;
	}
}

void readTrigrams(File &idxFile, Trigrams &trigrams)
{
	Index index;
	idxFile.seek(0);

	while (idxFile.readObj(index, false))
	{
		trigrams[index.trigram].push_back(index);
	}
}

size_t getLongestTrigramSize(const Trigrams &trigrams)
{
	size_t maxSize = 0;

	for (Trigrams::const_iterator it = trigrams.begin();
			it != trigrams.end(); ++it)
	{
		size_t size = 0;
		for (const Index &chunk: it->second)
			size += chunk.size;

		if (size > maxSize)
			maxSize = size;
	}

	return maxSize;
}

size_t readChunksData(const Chunks &chunks, File &dataFile, uint8_t *data,
		uint32_t &lastId)
{
	size_t read = 0;

	for (const Index &chunk : chunks)
	{
		dataFile.seek(chunk.offset);
		dataFile.read(data + read, chunk.size);

		read += chunk.size;
		lastId = chunk.lastId;
	}

	return read;
}

