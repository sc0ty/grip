#include "finder.h"
#include "compressedids.h"
#include "file.h"
#include "dir.h"
#include "config.h"
#include <set>
#include <cstdio>
#include <cstring>

using namespace std;


#define TRIGRAM_ENC(s)			(((s)[0] << 16) | ((s)[1] << 8) | (s)[2])
#define TRIGRAM_ENC3(a, b, c)	(((a) << 16) | ((b) << 8) | (c))

#define TO_LOWER(x)		((x)>='A' && (x)<='Z' ? (x)+'a'-'A' : (x))
#define TO_UPPER(x)		((x)>='a' && (x)<='z' ? (x)-'a'+'A' : (x))
#define IS_ALPHA(x)		(((x)>='a' && (x)<='z') || ((x)>='A' && (x)<='Z'))

inline uint32_t trigramToLower(uint32_t trigram)
{
	uint32_t res = 0;
	res |= TO_LOWER(trigram & 0xff);
	res |= TO_LOWER((trigram >> 8) & 0xff) << 8;
	res |= TO_LOWER((trigram >> 16) & 0xff) << 16;
	return res;
}


void getTrigramsDir(string &dir)
{
	if (dir.empty())
		getCurrentDirectory(dir);

	string gdir = dir + PATH_DELIMITER + GRIP_DIR;

	while (!directoryExists(gdir.c_str()))
	{
		size_t pos = dir.rfind(PATH_DELIMITER, dir.size());
		if ((pos == string::npos) || (pos == 0))
			throw FuncError("cannot find grip database");

		dir = dir.substr(0, pos);
		gdir = dir + PATH_DELIMITER + GRIP_DIR;
	}
}


Finder::Finder(const string &dirDb) : m_caseSensitive(true)
{
	string dir = dirDb;

	if (dir.empty())
		getTrigramsDir(dir);

	m_dataFile.open(dir + PATH_DELIMITER + TRIGRAMS_DATA_PATH, "r");
	File(dir + PATH_DELIMITER + TRIGRAMS_LIST_PATH, "r").readVector(m_indexes);
	m_fileList.read(dir + PATH_DELIMITER + FILE_LIST_PATH);
}

void Finder::caseSensitive(bool cs)
{
	m_caseSensitive = cs;
}

const Ids &Finder::get(uint32_t trigram)
{
	Chunks::const_iterator it = m_chunks.find(trigram);
	if (it != m_chunks.end())
		return it->second;

	Ids &ids = m_chunks[trigram];

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

void Finder::clearCache()
{
	m_chunks.clear();
}

void Finder::readChunks(const Index &index, Ids &ids)
{
	CompressedIds cids;

	m_dataFile.seek(index.offset);
	uint8_t *data = cids.setData(index.size, index.lastId);
	m_dataFile.read(data, index.size);

	cids.validate();
	cids.decompress(ids);
}

void Finder::find(const string &str, Ids &ids)
{
	if (str.size() < 3)
		throw ThisError("pattern too short").add("pattern", str);

	if (m_caseSensitive)
		findCaseSensitive(str, ids);
	else
		findCaseInsensitive(str, ids);

}

void Finder::findCaseSensitive(const string &str, Ids &ids)
{
	ids = get(TRIGRAM_ENC(str.c_str()));

	for (size_t i = 1; i <= str.size() - 3; i++)
	{
		if (ids.empty())
			break;

		const Ids &newIds = get(TRIGRAM_ENC(str.c_str()+i));
		ids.commonPart(newIds);
	}
}

struct Permutation
{
	uint8_t a, b;
	Ids ids;

	Permutation(uint8_t a) : a(a) {}
	Permutation(uint8_t a, uint8_t b) : a(a), b(b) {}

	bool operator== (const Permutation &p) const
	{
		return (a == p.a) && (b == p.b) && (ids == p.ids);
	}

	bool operator< (const Permutation &p) const
	{
		if (a != p.a) return a < p.a;
		if (b != p.b) return b < p.b;
		return ids < p.ids;
	}

	static void permute(set<Permutation> &res, uint8_t a, uint8_t b)
	{
		res.insert(Permutation(TO_LOWER(a), TO_LOWER(b)));
		res.insert(Permutation(TO_LOWER(a), TO_UPPER(b)));
		res.insert(Permutation(TO_UPPER(a), TO_LOWER(b)));
		res.insert(Permutation(TO_UPPER(a), TO_UPPER(b)));
	}
};

void Finder::findCaseInsensitive(const string &str, Ids &ids)
{
	if (str.size() < 3)
		throw ThisError("pattern too short").add("pattern", str);

	set<Permutation> perms, newPermutations;
	Permutation::permute(perms, str[0], str[1]);

	uint8_t c1 = TO_LOWER(str[2]);
	uint8_t c2 = TO_UPPER(str[2]);

	// initialize permutations set with first trigram
	for (const Permutation &perm : perms)
	{
		Permutation newPermutation(perm.b, c1);
		newPermutation.ids = get(TRIGRAM_ENC3(perm.a, perm.b, c1));
		if (!newPermutation.ids.empty())
			perms.insert(newPermutation);

		// add also upper case letter (if this is letter)
		if (c1 != c2)
		{
			newPermutation.ids = get(TRIGRAM_ENC3(perm.a, perm.b, c2));

			if (!newPermutation.ids.empty())
			{
				newPermutation.b = c2;
				perms.insert(newPermutation);
			}
		}
	}

	// permute other trigrams
	for (size_t i = 3; i < str.size(); i++)
	{
		uint8_t c1 = TO_LOWER(str[i]);
		uint8_t c2 = TO_UPPER(str[i]);

		// try to add next letter to existing set
		for (const Permutation &perm : perms)
		{
			Permutation newPermutation(perm.b, c1);

			{
				const Ids &newIds = get(TRIGRAM_ENC3(perm.a, perm.b, c1));
				newPermutation.ids.commonPart(perm.ids, newIds);

				if (!newPermutation.ids.empty())
					newPermutations.insert(newPermutation);
			}

			if (c1 != c2)
			{
				const Ids &newIds = get(TRIGRAM_ENC3(perm.a, perm.b, c2));
				newPermutation.ids.commonPart(perm.ids, newIds);

				if (!perm.ids.empty())
				{
					newPermutation.b = c2;
					newPermutations.insert(newPermutation);
				}
			}
		}

		clearCache();
		perms.swap(newPermutations);
		newPermutations.clear();

		if (perms.empty())
			break;
	}

	ids.clear();
	for (const Permutation &perm : perms)
		ids.merge(perm.ids);
}

const string &Finder::getFile(uint32_t id) const
{
	return m_fileList.get(id);
}

