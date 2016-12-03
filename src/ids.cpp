#include "ids.h"
#include "error.h"
#include <cstring>

using namespace std;


bool Ids::add(uint32_t id)
{
	if (m_ids.empty())
	{
		m_ids.push_back(id);
		return true;
	}

	uint32_t lastId = m_ids.back();
	if (id < lastId)
	{
			throw ThisError("out of order ID")
				.add("ID", id)
				.add("lastID", m_ids.back());
	}
	else if (id > lastId)
	{
		m_ids.push_back(id);
		return true;
	}
	else
	{
		return false;
	}
}

void Ids::clear()
{
	m_ids.clear();
}

size_t Ids::size() const
{
	return m_ids.size();
}

bool Ids::empty() const
{
	return m_ids.empty();
}

void Ids::swap(Ids &x)
{
	m_ids.swap(x.m_ids);
}

void Ids::merge(const Ids &ids1, const Ids &ids2)
{
	clear();

	const_iterator i1 = ids1.begin();
	const_iterator i2 = ids2.begin();

	while ((i1 != ids1.end()) && (i2 != ids2.end()))
		add(*i1 < *i2 ? *i1++ : *i2++);

	while (i1 != ids1.end())
		add(*i1++);

	while (i2 != ids2.end())
		add(*i2++);
}

void Ids::merge(const Ids &ids)
{
	Ids result;
	result.merge(*this, ids);
	swap(result);
}

void Ids::concat(const Ids &ids)
{
	m_ids.insert(m_ids.end(), ids.begin(),  ids.end());
}

bool Ids::hasId(uint32_t id) const
{
	size_t x = 0;
	size_t y = m_ids.size();

	if (y-- == 0)
		return false;

	while (x + 1 < y)
	{
		size_t pos = x + (y - x) / 2;
		uint32_t test = m_ids[pos];

		if (id < test)
			y = pos;
		else if (id > test)
			x = pos;
		else
			return true;
	}

	return (id == m_ids[x]) || (id == m_ids[y]);
}

void Ids::commonPart(const Ids &ids1, const Ids &ids2)
{
	clear();
	for (auto id : ids1.m_ids)
	{
		if (ids2.hasId(id))
			add(id);
	}
}

void Ids::commonPart(const Ids &ids)
{
	Ids result;
	result.commonPart(*this, ids);
	swap(result);
}

bool Ids::operator== (const Ids &ids) const
{
	size_t size = m_ids.size();

	if (size != ids.m_ids.size())
		return false;

	return memcmp(m_ids.data(), ids.m_ids.data(), size/4) == 0;
}

bool Ids::operator< (const Ids &ids) const
{
	size_t size = m_ids.size();

	if (size != ids.m_ids.size())
		return size < ids.m_ids.size();

	return memcmp(m_ids.data(), ids.m_ids.data(), size/4) < 0;
}

const uint32_t *Ids::getData() const
{
	return m_ids.data();
}

uint32_t *Ids::setData(size_t size)
{
	m_ids.resize(size);
	return m_ids.data();
}

Ids::iterator Ids::begin()
{
	return m_ids.begin();
}

Ids::iterator Ids::end()
{
	return m_ids.end();
}

Ids::const_iterator Ids::begin() const
{
	return m_ids.begin();
}

Ids::const_iterator Ids::end() const
{
	return m_ids.end();
}
