#include "node.h"
#include "case.h"
#include <cstring>

using namespace std;


static const char *skipBracket(const char *ch);


Node::Node(int val) : val(val)
{}

const list<NodePtr> &Node::getNext() const
{
	return next;
}

int Node::getVal() const
{
	return val;
}

void Node::parseFixedString(const string &exp, bool caseSensitive)
{
	if (exp.size() < 3)
		throw ThisError("pattern too short").add("pattern", exp);

	Node *node = addNext();
	node->tokenizeFixedString(exp.c_str());

	if (!caseSensitive)
	{
		node->markAlpha();
		node->permuteCaseMarked();
	}
}

void Node::tokenizeFixedString(const char *str)
{
	Node *node = this;
	for (const char *ch = str; *ch != '\0'; ch++)
		node = node->addNext(*ch);

	node->addNext(NODE_END);
}

void Node::parseRegex(const string &exp, bool extended, bool caseSensitive)
{
	Node *node = addNext();

	const char *e = exp.c_str();
	node->tokenizeRegex(&e, extended);

	if (!node->isUnambiguous())
	{
		throw ThisError("ambiguous regular expression - can't use with index")
			.add("regex", exp);
	}

	if (!caseSensitive)
	{
		node->markAlpha();
		node->permuteCaseMarked();
	}
}

Node *Node::tokenizeRegex(const char **exp, bool extended)
{
	Node *node = addNext();
	Node *prev = NULL;

	bool branched = false;

	const char *ch;

	for (ch = *exp; *ch != '\0'; ch++)
	{
		bool escaped = false;

		switch (*ch)
		{
			case '[':
				ch = skipBracket(ch);
				prev = NULL;
				node = node->addNext(NODE_SPLIT);
				break;

			case '.':
				node = node->addNext(NODE_SPLIT);
				break;

			case '*':
				if (prev)
				{
					prev->next.clear();
					node = prev->addNext(NODE_SPLIT);
					prev = NULL;
				}
				break;

			case '^':
				prev = node;
				node = node->addNext(NODE_SPLIT)->addNext('\n');
				break;

			case '$':
				prev = node;
				node = node->addNext('\n')->addNext(NODE_SPLIT);
				break;

			case '\\':
				if (*++ch == '\0')
				{
					throw ThisError("invalid regular expression")
						.add("expression", *exp);
				}

				if ((strchr("<>bBwW", *ch) != NULL) || (*ch >= '1' && *ch <= '9'))
				{
					prev = NULL;
					node = node->addNext(NODE_SPLIT);
					break;
				}

				escaped = true;

			default:
				if (extended ^ escaped)
				{
					switch (*ch)
					{
						case '{':
							for (ch++; *ch != '}' && *ch != '\0'; ch++);

						case '?':
							if (prev)
							{
								prev->next.clear();
								node = prev->addNext(NODE_SPLIT);
								prev = NULL;
							}
							break;

						case '+':
							node = node->addNext(NODE_SPLIT);
							break;

						case '|':
							prev = NULL;
							node = addNext();
							branched = true;
							break;

						case '(':
							ch++;
							prev = node;
							node = node->tokenizeRegex(&ch, extended);
							if (*ch != ')')
							{
								throw ThisError("invalid regular expression")
									.add("expression", *exp);
							}
							break;

						case ')':
							if (branched)
								node = addCommonDescendant(make_shared<Node>());

							*exp = ch;
							return node;
							break;

						default:
							prev = node;
							node = node->addNext(*ch);
					}
				}
				else
				{
					prev = node;
					node = node->addNext(*ch);
				}
		}
	}

	if (*ch == '\0')
		node = node->addNext(NODE_END);

	*exp = ch;
	return node;
}

void Node::markAlpha()
{
	if (IS_ALPHA(val))
		val |= NODE_MARK;

	for (auto n : next)
		n->markAlpha();
}

void Node::permuteCaseMarked()
{
	for (list<NodePtr>::iterator it = next.begin(); it != next.end(); it++)
	{
		Node *n = it->get();
		n->permuteCaseMarked();

		if (n->val & NODE_MARK)
		{
			n->val &= ~NODE_MARK;
			int newVal = NODE_EMPTY;

			if (IS_LOWER(n->val))
				newVal = TO_UPPER(n->val);
			else if (IS_UPPER(n->val))
				newVal = TO_LOWER(n->val);

			if (newVal != NODE_EMPTY)
			{
				NodePtr newNode = make_shared<Node>(newVal);
				newNode->next = n->next;
				next.insert(++it, newNode);
			}
		}
	}
}

Node *Node::addNext(int val)
{
	NodePtr n = make_shared<Node>(val);
	next.push_back(n);
	return n.get();
}

Node *Node::addCommonDescendant(NodePtr desc)
{
	if (next.empty())
	{
		next.push_back(desc);
	}
	else
	{
		for (auto n : next)
		{
			if (n.get() != desc.get())
				n->addCommonDescendant(desc);
		}
	}

	return desc.get();
}

void Node::findIds(Ids &res, DbReader &db) const
{
	Ids ids;
	findIds(res, ids, db, 0);
}

void Node::findIds(Ids &res, const Ids &ids, DbReader &db, uint32_t trigram) const
{
	bool gotNewTrigram = false;

	if (val == Node::NODE_END)
	{
		res.merge(ids);
	}
	else if (val == Node::NODE_SPLIT)
	{
		trigram = 0;
	}
	else if (val != NODE_EMPTY)
	{
		trigram <<= 8;
		trigram |= val & 0xff;
		trigram &= 0xffffff;
		gotNewTrigram = (trigram >= 0x10000);
	}

	if (gotNewTrigram)
	{
		const CompressedIds &cids = db.get(trigram);
		Ids nodeIds;
		cids.decompress(nodeIds);

		if (!nodeIds.empty())
		{
			if (ids.empty())
			{
				for (auto n : next)
					n->findIds(res, nodeIds, db, trigram);
			}
			else
			{
				Ids newIds;
				newIds.commonPart(ids, nodeIds);

				if (!newIds.empty())
				{
					for (auto n : next)
						n->findIds(res, newIds, db, trigram);
				}
			}
		}
	}
	else
	{
		for (auto n : next)
			n->findIds(res, ids, db, trigram);
	}
}

// check if there is at least three consecutive characters to generate trigram
bool Node::isUnambiguous(unsigned charsNo) const
{
	if (val == NODE_SPLIT)
	{
		charsNo = 0;
	}
	else if ((val != NODE_EMPTY) && (val != NODE_END))
	{
		if (++charsNo >= 3)
			return true;
	}

	for (auto n : next)
	{
		if (n->isUnambiguous(charsNo))
			return true;
	}

	return false;
}

string Node::toString(bool unique) const
{
	int v = val & ~NODE_MARK;
	string res;
	switch (v)
	{
		case NODE_EMPTY: res = "<EMPTY>"; break;
		case NODE_SPLIT: res = "<SPLIT>"; break;
		case NODE_END: res = "<END>"; break;
		case '\t': res = "'\\t'"; break;
		case '\n': res = "'\\n'"; break;
		case '\r': res = "'\\r'"; break;
		default:
			const static size_t size = 20;
			char buf[size];
			if (v >= 0x20 && v <= 0x7e)
				snprintf(buf, size, "'%c'", (char) v);
			else
				snprintf(buf, size, "%#x", v);
			res = buf;
	}

	if (unique)
	{
		const static size_t size = 40;
		char buf[size];
		snprintf(buf, size, "%p: %s", (void*) this, res.c_str());
		res = buf;
	}

	return res;
}

void Node::makeDotGraph(list<string> &graph) const
{
	bool header = graph.empty();
	if (header)
		graph.push_back("digraph Tree {");

	for (const auto &n : next)
		graph.push_back(string("\t\"") + toString(true) + "\" -> \""
				+ n->toString(true) + "\"");

	for (const auto &n : next)
		n->makeDotGraph(graph);

	if (header)
		graph.push_back("}");
}

const char *skipBracket(const char *ch)
{
	int lvl = 1;
	do
	{
		switch(*++ch)
		{
			case '[':
				lvl++;
				break;

			case ']':
				lvl--;
				break;

			case '\\':
				ch++;

			default:
				if (*ch == '\0')
				{
					throw FuncError("invalid regular expression")
						.add("expression", *ch);
				}
		}
	}
	while (lvl > 0);

	return ch;
}
