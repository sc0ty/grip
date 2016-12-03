#ifndef __FINDER_H__
#define __FINDER_H__

#include "index.h"
#include "ids.h"
#include "filelist.h"
#include "file.h"
#include <vector>
#include <string>
#include <stdint.h>


void getTrigramsDir(std::string &dir);


class Finder
{
	public:
		Finder(const std::string &dir = "");

		const Ids &get(uint32_t trigram);
		void clearCache();

		void find(const std::string &str, Ids &ids);

		void caseSensitive(bool sensitive);

		const std::string &getFile(uint32_t id) const;

	private:
		void readChunks(const Index &index, Ids &ids);

		void findCaseSensitive(const std::string &str, Ids &ids);
		void findCaseInsensitive(const std::string &str, Ids &ids);

	private:
		std::vector<Index> m_indexes;
		File m_dataFile;
		Files m_fileList;

		typedef std::map<uint32_t /* trigram */, Ids> Chunks;
		Chunks m_chunks;

		bool m_caseSensitive;
};

#endif
