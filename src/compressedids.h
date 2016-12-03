#ifndef __COMPRESSED_IDS_H__
#define __COMPRESSED_IDS_H__

#include "ids.h"
#include <cassert>


class CompressedIds
{
	public:
		CompressedIds();

		/** Add single ID, must be greater or equal than lastId() */
		unsigned add(uint32_t id);

		void clear();

		size_t size() const;
		bool empty() const;

		void swap(CompressedIds &ids);

		/** move content of ids, ids will be cleared */
		void move(CompressedIds &ids);

		bool hasId(uint32_t id) const;

		void decompress(Ids &ids) const;
		void commonPart(Ids &ids) const;

		uint32_t firstId() const;
		uint32_t lastId() const;

		const uint8_t *getData() const;
		uint8_t *setData(size_t size, uint32_t lastId);
		uint8_t *appendData(size_t size, uint32_t lastId);
		void validate() const;

		void clearChunk();

	public:
		class iterator
		{
			public:
				iterator();
				iterator(const uint8_t *data, size_t size);
				iterator(const uint8_t *data);

				uint32_t operator* () const
				{
					assert(m_pos < m_end);
					return m_val;
				}

				iterator &operator++();

				bool end() const
				{
					return m_pos >= m_end;
				}

				bool operator!= (const iterator &it) const
				{
					return m_pos != it.m_pos;
				}

			private:
				const uint8_t *m_pos;
				const uint8_t *m_end;

				uint32_t m_val;
				uint32_t m_delta;
				unsigned m_deltaRep;
		};

		iterator begin() const;
		iterator end() const;

	private:
		std::vector<uint8_t> m_ids;
		uint32_t m_lastId;

		uint32_t m_lastDelta;
		uint32_t m_lastDeltaRep;
};

#endif
