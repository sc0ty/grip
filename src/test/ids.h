#ifndef __TEST_IDS_H__
#define __TEST_IDS_H__

#include <vector>
#include <cstdint>
#include "compressedids.h"
#include "error.h"


template <typename T>
bool compareIds(const T &ids, const std::vector<uint32_t> &items)
{
	ids.validate();
	typename std::vector<uint32_t>::const_iterator it = items.begin();

	for (auto id : ids)
	{
		if ((it == items.end()) || (id != *it))
			return false;

		++it;
	}

	return it == items.end();
}

#define IDS(...) std::vector<uint32_t>({ __VA_ARGS__ })
#define CMP_IDS(ids, ...) compareIds(ids, IDS( __VA_ARGS__ ))

#endif
