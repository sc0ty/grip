#include <boost/filesystem.hpp>
#include "dbreader.h"
#include "grep.h"
#include "pattern.h"
#include "glob.h"
#include "fileline.h"
#include "dir.h"
#include "config.h"
#include "print.h"
#include "error.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <climits>

extern "C" {
#include "getopt.h"
}

using namespace std;


enum
{
	COLOR_OPTION = CHAR_MAX + 1,
	EXCLUDE_OPTION,
	EXCLUDE_FROM_OPTION,
	INCLUDE_OPTION,
	EXTENDED_GLOB_OPTION,
	DOT_GRAPH_OPTION,
	GLOB_TYPE_OPTIONS,			// must be last one
};


static struct option const LONGOPTS[] =
{
#ifndef USE_MATCHER
	{"extended-regex", no_argument, NULL, 'E'},
	{"fixed-strings", no_argument, NULL, 'F'},
	{"basic-regex", no_argument, NULL, 'G'},
#endif

	{"after-context", required_argument, NULL, 'A'},
	{"before-context", required_argument, NULL, 'B'},
	{"context", required_argument, NULL, 'C'},
	{"color", optional_argument, NULL, COLOR_OPTION},
	{"colour", optional_argument, NULL, COLOR_OPTION},
	{"dot", required_argument, NULL, DOT_GRAPH_OPTION},
	{"help", no_argument, NULL, 'h'},
	{"ignore-case", optional_argument, NULL, 'i'},
	{"exclude", required_argument, NULL, EXCLUDE_OPTION},
	{"exclude-from", required_argument, NULL, EXCLUDE_FROM_OPTION},
	{"file", required_argument, NULL, 'f'},
	{"global", no_argument, NULL, 'g'},
	{"include", required_argument, NULL, INCLUDE_OPTION},
#if defined(FNM_EXTMATCH)
	{"extended-glob", no_argument, NULL, EXTENDED_GLOB_OPTION},
#endif
	{"list", no_argument, NULL, 'l'},
	{"no-messages", no_argument, NULL, 's'},
	{"word-regexp", no_argument, NULL, 'w'},
	{"line-regexp", no_argument, NULL, 'x'},
	{"version", no_argument, NULL, 'V'},

#define GLOB_TYPE_FILTER(id, name, ...) \
	{#name, no_argument, NULL, GLOB_TYPE_OPTIONS + id*2}, \
	{"no" #name, no_argument, NULL, GLOB_TYPE_OPTIONS + id*2 + 1},
#include "globfilters.def"
#undef GLOB_TYPE_FILTER

	{NULL, 0, NULL, 0}
};

static char const SHORTOPTS[] =
#ifndef USE_MATCHER
	"EFG"
#endif
	"A:B:C:f:ghilswxV0123456789";

static void readPatternsFromFile(const char *fname, vector<string> &patterns);
static void readExcludeFromFile(Glob &glob, const char *fname);
static void saveDotGraph(Node *tree, const char *fname);
static void usage(const char *name);
static void version(const char *name);


int main(int argc, char * const argv[])
{
	try
	{
		color::init();

		vector<string> patterns;
		bool caseSensitivePatterns = true;

#ifdef USE_MATCHER
		Pattern::Mode mode = USE_MATCHER;
#else
		Pattern::Mode mode = Pattern::BASIC;
#endif

		Grep grep;
		unsigned context = 0;

		Glob glob;

		bool listOnly = false;
		int verbose = 1;
		bool global = false;

		const char *dotGraphPath = NULL;

		int opt;
		while ((opt = getopt_long(argc, argv, SHORTOPTS, LONGOPTS, NULL)) != -1)
		{
			if (opt >= '0' && opt <= '9')
			{
				context *= 10;
				context += opt - '0';
				continue;
			}

			switch (opt)
			{
				case 'A':
					grep.setAfterContext(atoi(optarg));
					break;

				case 'B':
					grep.setBeforeContext(atoi(optarg));
					break;

				case 'C':
					grep.setBeforeContext(atoi(optarg));
					grep.setAfterContext(atoi(optarg));
					break;

				case COLOR_OPTION:
					if ((optarg == NULL) || (strcmp(optarg, "always") == 0))
						color::mode(true);
					else if (strcmp(optarg, "never") == 0)
						color::mode(false);
					break;

				case DOT_GRAPH_OPTION:
					dotGraphPath = optarg;
					break;

				case 'f':
					readPatternsFromFile(optarg, patterns);
					break;

				case 'g':
					global = true;
					break;

				case 'i':
					if ((optarg != NULL) && (strcmp(optarg, "all") == 0))
						optarg = NULL;

					if ((optarg == NULL) || (strcmp(optarg, "pattern") == 0))
						caseSensitivePatterns = false;
					if ((optarg == NULL) || (strcmp(optarg, "glob") == 0))
						glob.caseSensitive(false);
					break;

#ifndef USE_MATCHER
				case 'E':
					mode = Pattern::EXTENDED;
					break;

				case 'F':
					mode = Pattern::FIXED;
					break;

				case 'G':
					mode = Pattern::BASIC;
					break;
#endif

				case 'l':
					listOnly = true;
					break;

				case 's':
					verbose = 0;
					break;

				case 'w':
					grep.matchMode(Grep::MATCH_WHOLE_WORD);
					break;

				case 'x':
					grep.matchMode(Grep::MATCH_WHOLE_LINE);
					break;

				case 'V':
					version(argv[0]);
					return 0;
					break;

				case EXCLUDE_OPTION:
					glob.addExcludePattern(optarg);
					break;

				case EXCLUDE_FROM_OPTION:
					readExcludeFromFile(glob, optarg);
					break;

				case INCLUDE_OPTION:
					glob.addIncludePattern(optarg);
					break;

#if defined(FNM_EXTMATCH)
				case EXTENDED_GLOB_OPTION:
					glob.extendedMatch(true);
					break;
#endif

#define GLOB_TYPE_FILTER(id, name, ...) \
				case GLOB_TYPE_OPTIONS + id*2: \
				{ \
					static const char *patterns[] = { __VA_ARGS__ }; \
					for (size_t i = 0; i < sizeof(patterns)/sizeof(char*); i++) \
						glob.addIncludePattern(patterns[i]); \
					break; \
				} \
				case GLOB_TYPE_OPTIONS + id*2 + 1: \
				{ \
					static const char *patterns[] = { __VA_ARGS__ }; \
					for (size_t i = 0; i < sizeof(patterns)/sizeof(char*); i++) \
						glob.addExcludePattern(patterns[i]); \
					break; \
				} \

#include "globfilters.def"
#undef GLOB_TYPE_FILTER

				case 'h':
				default:
					usage(argv[0]);
					return 0;
			}
		}

		if (context > 0)
		{
			grep.setBeforeContext(context);
			grep.setAfterContext(context);
		}

		for (int i = optind; i < argc; i++)
			patterns.push_back(argv[i]);

		if (patterns.empty())
		{
			usage(argv[0]);
			return 2;
		}

		string dbdir = getIndexPath();
		string cwd = getCurrentDirectory();

		DbReader database(dbdir);
		Ids ids;
		Node tree;

		for (const string &pattern : patterns)
		{
			Pattern *p = Pattern::create(pattern, mode, caseSensitivePatterns);
			grep.addPattern(p);
			p->tokenize(tree);
		}

		if (dotGraphPath)
			saveDotGraph(&tree, dotGraphPath);

		tree.findIds(ids, database);
		bool found = false;

		for (auto id : ids)
		{
			string filePath = database.getFile(id);
			if (!isAbsolutePath(filePath))
				filePath = string(dbdir + PATH_DELIMITER) + filePath;
			canonizePath(filePath);
			if ((global || isInDirectory(cwd, filePath)) && glob.compare(filePath))
			{
				if (!global)
					filePath = getRelativePath(cwd, filePath);

				if (listOnly)
				{
					puts(filePath.c_str());
				}
				else
				{
					try
					{
						found |= grep.grepFile(filePath);
					}
					catch (const Error &ex)
					{
						if (verbose >= 1)
							fprintf(stderr, "%s: %s\n", filePath.c_str(), ex.what());
					}
				}
			}
		}

		return found ? 0 : 1;
	}
	catch (const Error &ex)
	{
		fprintf(stderr, "error: %s\n", ex.what());
		for (auto tag : ex.tags)
			fprintf(stderr, "\t%s: %s\n", tag.first.c_str(), tag.second.c_str());

		if (ex.get("type") == "invalid_query")
			return 3;

		return 2;
	}
}

void readPatternsFromFile(const char *fname, vector<string> &patterns)
{
	FileLineReader file(fname);
	string line;
	while (file.readLine(line))
		patterns.push_back(line);
}

void readExcludeFromFile(Glob &glob, const char *fname)
{
	try
	{
		FileLineReader file(fname);
		string line;

		while (file.readLine(line))
			glob.addExcludePattern(line);
	}
	catch (const Error &ex)
	{
		fprintf(stderr, "error: invalid exclude file \"%s\"\n", fname);
		throw;
	}
}

void saveDotGraph(Node *tree, const char *fname)
{
	string graph;
	tree->makeDotGraph(graph);

	File fp(fname, "w");
	fp.writeLine(graph);
}

void usage(const char *name)
{
	printf("Usage: %s [OPTIONS] PATTERN [PATTERN...]\n"
	"Indexed grep - search for PATTERN in previously indexed files "
	"(with gripgen).\n"
	"\n"
	"Regexp selection and interpretation:\n"
#ifndef USE_MATCHER
	"  -E, --extended-regexp     PATTERN is an extended regular expression\n"
	"  -F, --fixed-strings       PATTERN is a strings\n"
	"  -G, --basic-regexp        PATTERN is a basic regular expression (default)\n"
#endif
	"  -f, --file=FILE           obtain PATTERN from FILE\n"
	"  -g, --global              show all matches, including parent directories\n"
	"  -i, --ignore-case[=WHERE] ignore case distinction in PATTERN\n"
	"                            WHERE is 'pattern', 'glob' or 'all' (default)\n"
	"  -w, --word-regexp         force PATTERN to match only whole words\n"
	"  -x, --line-regexp         force PATTERN to match only whole lines\n"
	"\n"
	"Miscellaneous:\n"
	"  -s, --no-messages         suppress error messages\n"
	"  -h, --help                display this help and exit\n"
	"\n"
	"Output control:\n"
	"      --include=GLOB        search only files that match GLOB pattern\n"
	"      --exclude=GLOB        skip files and directories matching GLOB pattern\n"
	"      --exclude-from=FILE   skip files matching any file pattern from FILE\n"
#if defined(FNM_EXTMATCH)
	"      --extended-glob       use ksh-like extended match for globbing\n"
#endif
	"  -l, --list                only list files with potential match\n"
	"\n"
	"Context control:\n"
	"  -B, --before-context=NUM  print NUM lines of leading context\n"
	"  -A, --after-context=NUM   print NUM lines of trailing context\n"
	"  -C, --context=NUM         print NUM lines of output context\n"
	"  -NUM                      same as --context=NUM\n"
	"      --color[=WHEN],\n"
	"      --colour[=WHEN]       use markers to highlight the matching strings;\n"
	"                            WHEN is 'always', 'never', or 'auto' (default)\n"
	"\n"
	"Debug options:\n"
	"  --dot=FILE                save query as Graphviz DOT graph\n",
	name);
}

void version(const char *name)
{
	printf("%s (index based grep) " VERSION_STR "\n"
	"Copyright (C) 2016 Free Software Foundation, Inc.\n"
	"License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
	"This is free software: you are free to change and redistribute it.\n"
	"There is NO WARRANTY, to the extent permitted by law.\n"
	"\n"
	"Written by Mike Szymaniak, http://sc0ty.pl\n",
	name);
}

