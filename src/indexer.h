#ifndef __INDEXER_H__
#define __INDEXER_H__

#include <vector>
#include <string>
#include <stdint.h>
#include "file.h"
#include "compressedids.h"


class Indexer
{
	public:
		Indexer(size_t bufferSize = 4*1024*1024);
		~Indexer();

		void open(const std::string &dir = ".");
		void close();

		bool indexFile(const std::string &fname);

		size_t size() const;
		size_t filesNo() const;
		size_t filesTotalSize() const;
		size_t chunksSize() const;

		void write();
		void sortDatabase();

	private:
		uint32_t addFile(const std::string &fname);
		void addTrigram(uint32_t trigram, uint32_t fileId);
		void freeIds();

	private:
		CompressedIds **m_ids;
		size_t m_size;
		size_t m_filesTotalSize;
		size_t m_chunksSize;

		std::string m_fileList;
		uint32_t m_fileId;

		std::vector<uint8_t> m_buffer;

		File m_idxFile;
		File m_dataFile;
		File m_filesFile;
		std::string m_dir;
};

#endif
