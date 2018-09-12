#include "ids.h"
#include "error.h"
#include <cstring>
#include <algorithm>

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
	m_ids.resize(ids1.size() + ids2.size());
	auto begin = m_ids.begin();
	auto end = set_union(ids1.begin(), ids1.end(), ids2.begin(), ids2.end(), begin);
	m_ids.resize(end - begin);
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
	return binary_search(m_ids.begin(), m_ids.end(), id);
}

void Ids::commonPart(const Ids &ids1, const Ids &ids2)
{
	m_ids.resize(max(ids1.size(), ids2.size()));
	auto begin = m_ids.begin();
	auto end = set_intersection(ids1.begin(), ids1.end(), ids2.begin(), ids2.end(), begin);
	m_ids.resize(end - begin);
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

void Ids::validate() const
{
	if (!is_sorted(m_ids.begin(), m_ids.end()))
		throw ThisError("internal error, ids not sorted");
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
