#ifndef __NODE_H__
#define __NODE_H__

#include <memory>
#include <list>
#include <string>
#include <climits>
#include "dbreader.h"
#include "ids.h"


class Node;
typedef std::shared_ptr<Node> NodePtr;


class Node
{
	public:
		enum Value
		{
			NODE_EMPTY = CHAR_MAX + 1,
			NODE_SPLIT,
			NODE_END,
			NODE_MARK = INT_MAX - (INT_MAX / 2),
		};

	public:
		Node(int val = NODE_EMPTY);

		void parseFixedString(const std::string &exp, bool caseSensitive);
		void parseRegex(const std::string &exp, bool extended, bool caseSensitive);

		bool isUnambiguous(unsigned charsNo = 0) const;
		void findIds(Ids &res, DbReader &db) const;

		const std::list<NodePtr> &getNext() const;
		int getVal() const;

	private:
		void tokenizeFixedString(const char *str);
		Node *tokenizeRegex(const char **exp, bool extended);

		void markAlpha();
		void permuteCaseMarked();

		void findIds(Ids &res, const Ids &ids, DbReader &db, uint32_t trigram) const;

		Node *addNext(int val = NODE_EMPTY);
		Node *addCommonDescendant(NodePtr desc);

	private:
		int val;
		std::list<NodePtr> next;
};

#endif
