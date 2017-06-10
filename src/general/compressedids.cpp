#include "compressedids.h"
#include "error.h"

using namespace std;

#define SWAP_VAL(x, y)		{ auto tmp = (x); (x) = (y); (y) = tmp; }


/*** CompressedIds ***/

CompressedIds::CompressedIds()
	: m_lastId(0), m_lastDelta((uint32_t) -1)
{}

unsigned CompressedIds::add(uint32_t id)
{
	unsigned added = 0;

	if (m_lastId == (uint32_t) -1)
		m_lastId = lastId();

	if (id < m_lastId)
		throw ThisError("out of order ID").add("ID", id).add("lastID", m_lastId);

	if ((id != m_lastId) || m_ids.empty())
	{
		uint32_t delta = id - m_lastId;
		if (m_lastDelta == (uint32_t) -1)
			m_lastDelta = getLastDelta();

		if ((m_lastDelta != (uint32_t) -1) && (delta == m_lastDelta))
		{
			if (m_ids.back() >= 0x40 && m_ids.back() < 0x7f)
			{
				m_ids.back()++;
			}
			else
			{
				m_ids.push_back(0x41);
				added = 1;
			}

			m_lastId = id;
		}
		else
		{
			if (delta >> 27)
			{
				m_ids.push_back(((delta >> 27) & 0x7f) | 0x80);
				added++;
			}
			if (delta >> 20)
			{
				m_ids.push_back(((delta >> 20) & 0x7f) | 0x80);
				added++;
			}
			if (delta >> 13)
			{
				m_ids.push_back(((delta >> 13) & 0x7f) | 0x80);
				added++;
			}
			if (delta >> 6)
			{
				m_ids.push_back(((delta >> 6)  & 0x7f) | 0x80);
				added++;
			}
			m_ids.push_back(delta & 0x3f);
			added++;

			m_lastId = id;
			m_lastDelta = delta;
		}
	}

	return added;
}

void CompressedIds::clear()
{
	m_ids.clear();
	m_lastId = 0;
	m_lastDelta = (uint32_t) -1;
}

size_t CompressedIds::size() const
{
	return m_ids.size();
}

bool CompressedIds::empty() const
{
	return m_ids.empty();
}

void CompressedIds::swap(CompressedIds &ids)
{
	m_ids.swap(ids.m_ids);
	SWAP_VAL(m_lastId, ids.m_lastId);
	SWAP_VAL(m_lastDelta, ids.m_lastDelta);
}

void CompressedIds::move(CompressedIds &ids)
{
	m_lastId = ids.m_lastId;
	m_lastDelta = ids.m_lastDelta;

	m_ids.swap(ids.m_ids);
	ids.clear();
}

bool CompressedIds::hasId(uint32_t id) const
{
	for (iterator it = begin(); !it.end(); ++it)
	{
		if (*it == id)
			return true;
		else if (*it > id)
			return false;
	}

	return false;
}

void CompressedIds::decompress(Ids &ids) const
{
	for (iterator it = begin(); !it.end(); ++it)
		ids.add(*it);
}

void CompressedIds::commonPart(Ids &ids) const
{
	Ids result;
	for (iterator it = begin(); !it.end(); ++it)
	{
		if (ids.hasId(*it) > 0)
			result.add(*it);
	}
	ids.swap(result);
}

uint32_t CompressedIds::firstId() const
{
	return *begin();
}

uint32_t CompressedIds::lastId() const
{
	if (m_lastId != (uint32_t) -1)
		return m_lastId;

	uint32_t lastId = 0;
	for (uint32_t id : *this)
		lastId = id;
	return lastId;
}

const uint8_t *CompressedIds::getData() const
{
	return m_ids.data();
}

uint8_t *CompressedIds::setData(size_t size, uint32_t lastId)
{
	m_lastId = lastId;
	m_lastDelta = (uint32_t) -1;

	m_ids.resize(size);
	return m_ids.data();
}

uint8_t *CompressedIds::appendData(size_t size, uint32_t lastId)
{
	m_lastId = lastId;
	m_lastDelta = (uint32_t) -1;

	size_t pos = m_ids.size();
	m_ids.resize(pos + size);
	return m_ids.data() + pos;
}

void CompressedIds::validate() const
{
	if (!m_ids.empty())
	{
		if ((m_ids.front() & 0xc0) == 0x40)
			throw ThisError("malformed database, inconsistent chunk data");

		if (m_ids.back() & 0x80)
			throw ThisError("malformed database, incomplete chunk data");
	}
}

void CompressedIds::clearChunk()
{
	m_ids.clear();
}

uint32_t CompressedIds::getLastDelta() const
{
	if (m_ids.empty())
		return (uint32_t) -1;

	size_t pos = m_ids.size();

	// skip repetitions
	while (pos > 0 && m_ids[pos] & 0x40)
		pos--;

	uint32_t delta = m_ids[pos];
	uint32_t offset = 6;
	while (pos > 0 && !(m_ids[--pos] & 0x80))
	{
		delta |= ((uint32_t) m_ids[pos] & 0x7f) << offset;
		offset += 7;
	}

	return delta;
}


CompressedIds::iterator CompressedIds::begin() const
{
	return CompressedIds::iterator(m_ids.data(), m_ids.size());
}

CompressedIds::iterator CompressedIds::end() const
{
	return CompressedIds::iterator(m_ids.data() + m_ids.size());
}


CompressedIds::iterator::iterator()
	: m_pos(NULL), m_end(NULL), m_val(0), m_delta(0), m_deltaRep(0)
{}

CompressedIds::iterator::iterator(const uint8_t *data, size_t size) :
	m_pos(data - 1),
	m_end(data + size),
	m_val(0),
	m_delta(0),
	m_deltaRep(0)
{
	operator++();
}

CompressedIds::iterator::iterator(const uint8_t *data) :
	m_pos(data),
	m_end(data)
{}

CompressedIds::iterator &CompressedIds::iterator::operator++()
{
	if (m_deltaRep > 0)
	{
		m_val += m_delta;
		--m_deltaRep;
		return *this;
	}

	m_pos++;
	if (m_pos >= m_end)
	{
		return *this;
	}
	else if ((*m_pos & 0xc0) == 0x40)
	{
		m_val += m_delta;
		unsigned rep = (*m_pos & 0x3f);
		if (rep > 1)
		{
			m_deltaRep = rep - 1;
		}
		else if (rep == 1)
		{
			m_deltaRep = 0;
		}
		else // rep == 0
		{
			operator++();
		}
	}
	else
	{
		uint32_t d = 0;

		while (*m_pos & 0x80)
		{
			d <<= 7;
			d |= *m_pos & 0x7f;

			if (++m_pos >= m_end)
				throw ThisError("malformed database");
		}

		d <<= 6;
		d |= *m_pos & 0x3f;

		m_val += d;
		m_delta = d;
	}

	return *this;
}
