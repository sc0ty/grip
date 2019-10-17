#ifndef __DB_READER_H__
#define __DB_READER_H__

#include "index.h"
#include "compressedids.h"
#include "filelist.h"
#include "file.h"
#include <vector>
#include <map>
#include <string>
#include <stdint.h>


class DbReader
{
	public:
		DbReader(const std::string &dir = "");

		const CompressedIds &get(uint32_t trigram);
		const std::vector<Index> &getIndexes() const;

		void clearCache();

		const std::string &getFile(uint32_t id) const;
		uint32_t getFilesNo() const;

	private:
		void readChunks(const Index &index, CompressedIds &ids);

	private:
		std::vector<Index> m_indexes;
		File m_dataFile;
		Files m_fileList;

		typedef std::map<uint32_t /* trigram */, CompressedIds> Chunks;
		Chunks m_chunks;
};

#endif
