#include "finder.h"
#include "grep.h"
#include "dir.h"
#include "error.h"
#include <cstring>
#include <cstdio>
#include <climits>
#include <unistd.h>
#include <getopt.h>

using namespace std;


enum
{
	COLOR_OPTION = CHAR_MAX + 1,
};


static struct option const LONGOPTS[] =
{
	{"after-context", required_argument, NULL, 'A'},
	{"before-context", required_argument, NULL, 'B'},
	{"context", required_argument, NULL, 'C'},
	{"color", optional_argument, NULL, COLOR_OPTION},
	{"colour", optional_argument, NULL, COLOR_OPTION},
	{"help", no_argument, NULL, 'h'},
	{"ignore-case", no_argument, NULL, 'i'},
	{"list", no_argument, NULL, 'l'},
	{"no-messages", no_argument, NULL, 's'},
	{"word-regexp", no_argument, NULL, 'w'},
	{NULL, 0, NULL, 0}
};

static char const SHORTOPTS[] = "A:B:C:hilsw";

static void usage(const char *name);


int main(int argc, char * const argv[])
{
	try
	{
		string dbdir, cwd;

		getTrigramsDir(dbdir);
		getCurrentDirectory(cwd);

		Finder finder(dbdir);
		Ids ids;

		Grep grep;
		grep.outputFormat(isatty(STDOUT_FILENO));

		bool listOnly = false;
		int verbose = 1;

		int opt;
		while ((opt = getopt_long(argc, argv, SHORTOPTS, LONGOPTS, NULL)) != -1)
		{
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
						grep.outputFormat(true);
					else if (strcmp(optarg, "never") == 0)
						grep.outputFormat(false);
					break;

				case 'i':
					finder.caseSensitive(false);
					grep.caseSensitive(false);
					break;

				case 'l':
					listOnly = true;
					break;

				case 's':
					verbose = 0;
					break;

				case 'w':
					grep.wholeWordMatch(true);
					break;

				case 'h':
				default:
					usage(argv[0]);
					return 0;
			}
		}

		if (optind >= argc)
		{
			usage(argv[0]);
			return 2;
		}

		const char *pattern = argv[optind];
		finder.find(pattern, ids);

		grep.setPattern(pattern);

		bool found = false;
		for (auto id : ids)
		{
			string filePath;
			canonizePath(dbdir + PATH_DELIMITER + finder.getFile(id), filePath);
			if (isInDirectory(cwd, filePath))
			{
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

		return 2;
	}
}

void usage(const char *name)
{
	printf("Usage: %s [OPTIONS] PATTERN\n"
	"Indexed grep - search for PATTERN in previously indexed files "
	"(with gripgen).\n"
	"Author: Mike Szymaniak, http://sc0ty.pl\n"
	"\n"
	"Options:\n"
	"  -i, --ignore-case         ignore case distinction in PATTERN\n"
	"  -w, --word-regexp         force PATTERN to match only whole words\n"
	"  -B, --before-context=NUM  print NUM lines of leading context\n"
	"  -A, --after-context=NUM   print NUM lines of trailing context\n"
	"  -C, --context=NUM         print NUM lines of output context\n"
	"  -l, --list                only list files with potential match\n"
	"      --color[=WHEN],\n"
	"      --colour[=WHEN]       use markers to highlight the matching strings;\n"
	"                            WHEN is 'always', 'never', or 'auto'\n"
	"  -s, --no-messages         supress error messages\n"
	"  -h, --help                display this help and exit\n",
	name);
}

