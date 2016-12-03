#ifndef __INDEX_H__
#define __INDEX_H__

#include <cstddef>
#include <stdint.h>


struct Index
{
	uint32_t trigram;
	uint32_t lastId;
	size_t offset;
	size_t size;

	inline Index() {}

	inline Index(uint32_t trigram, size_t offset, size_t size, uint32_t lastId)
		: trigram(trigram), lastId(lastId), offset(offset), size(size) {}

	inline bool isValid() const
	{
		return (trigram & 0xff000000) == 0;
	}

	inline void invalidate()
	{
		trigram = 0xffffffff;
	}

	bool operator< (const Index &index) const
	{
		return (trigram < index.trigram) ||
			((trigram == index.trigram) && (lastId < index.lastId));
	}
};


#endif

