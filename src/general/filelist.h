#ifndef __FILES_H__
#define __FILES_H__

#include <cstdint>
#include <vector>
#include <string>


class Files
{
	public:
		uint32_t add(const char *fname);
		uint32_t getNextId() const;

		void clear();

		const std::string &get(uint32_t id) const;
		uint32_t size() const;
		bool empty() const;

		void write(const std::string &fname) const;
		void read(const std::string &fname);

		typedef std::vector<std::string>::iterator iterator;
		iterator begin();
		iterator end();

		typedef std::vector<std::string>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

	private:
		std::vector<std::string> m_files;
};

#endif
