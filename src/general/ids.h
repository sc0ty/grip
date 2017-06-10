#ifndef __IDS_H__
#define __IDS_H__

#include <vector>
#include <cstddef>
#include <cstdint>


class Ids
{
	public:
		bool add(uint32_t id);
		void clear();

		size_t size() const;
		bool empty() const;

		void swap(Ids &x);

		void merge(const Ids &ids1, const Ids &ids2);
		void merge(const Ids &ids);

		void concat(const Ids &ids);

		bool hasId(uint32_t id) const;
		void commonPart(const Ids &ids1, const Ids &ids2);
		void commonPart(const Ids &ids);

		bool operator== (const Ids &ids) const;
		bool operator< (const Ids &ids) const;

		const uint32_t *getData() const;
		uint32_t *setData(size_t size);

		typedef std::vector<uint32_t>::iterator iterator;
		iterator begin();
		iterator end();

		typedef std::vector<uint32_t>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		void validate() const;
	private:
		std::vector<uint32_t> m_ids;
};

#endif
