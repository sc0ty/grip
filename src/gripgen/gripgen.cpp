#include "indexer.h"
#include "dir.h"
#include "fileline.h"
#include "config.h"
#include "print.h"
#include "error.h"
#include <chrono>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <climits>

extern "C" {
#include "getopt.h"
#include "getopt.c"
#include "getopt1.c"
}

using namespace std;
using namespace std::chrono;

enum
{
	CHUNK_SIZE_OPTION = CHAR_MAX + 1,
};


static struct option const LONGOPTS[] =
{
	{"update", no_argument, NULL, 'u'},
	{"chunk-size", required_argument, NULL, CHUNK_SIZE_OPTION},
	{"verbose", optional_argument, NULL, 'v'},
	{"quiet", no_argument, NULL, 'q'},
	{"silent", no_argument, NULL, 'q'},
	{"no-messages", no_argument, NULL, 's'},
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'V'},
	{NULL, 0, NULL, 0}
};

static char const SHORTOPTS[] = "hqsuvV";


static void printProgress(const Indexer &indexer, unsigned long chunksNo, const string &fileName);
static void printFileError(const Error &err, const char *fname);
static void printGenericError(const Error &er);
static void usage(const char *name);
static void version(const char *name);


static steady_clock::time_point startTime, lastTime;

static bool supressErrors = false;
static int result;


int main(int argc, char * const argv[])
{
	Indexer indexer;
	size_t chunkSize = 64 * 1024 * 1024;

	FileLineReader files;

	int verbose = 1;
	bool updateIndex = false;

	result = 0;

	try
	{
		int opt;
		while ((opt = getopt_long(argc, argv, SHORTOPTS, LONGOPTS, NULL)) != -1)
		{
			switch (opt)
			{
				case 's':
					supressErrors = true;
					break;

				case 'u':
					updateIndex = true;
					break;

				case 'v':
					if (optarg != NULL)
						verbose = atoi(optarg);
					else
						verbose++;
					break;

				case 'q':
					verbose = 0;
					supressErrors = true;
					break;

				case CHUNK_SIZE_OPTION:
					chunkSize = atol(optarg) * 1024 * 1024;
					break;

				case 'V':
					version(argv[0]);
					return 0;
					break;

				case 'h':
				default:
					usage(argv[0]);
					return result;
			}
		}

		if (optind < argc)
		{
			if (verbose >= 2)
				println("reading list from file %s", argv[optind]);
			files.open(argv[optind]);
		}
		else if (updateIndex)
		{
			if (verbose >= 2)
				println("updating existing index (\"" FILE_LIST_PATH "\")");
			files.open(FILE_LIST_PATH);
		}
		else
		{
			if (verbose >= 2)
				println("reading list from standard input");
			files.open(stdin);
		}

		if (verbose >= 2)
		{
			println("max chunk size: %zu MB",
					chunkSize / (1024*1024));
		}

		if (verbose >= 1)
			print("indexing...");

		indexer.open();

		startTime = lastTime = steady_clock::now();

		unsigned long chunksNo = 0;
		unsigned long filesNo = 0;
		string fileName;
		const char *fname;

		try
		{
			while ((fname = files.readLine(false)) != NULL)
			{
				try
				{
					if (strncmp(fname, GRIP_DIR, sizeof(GRIP_DIR) - 1) == 0)
						continue;

					if (strstr(fname, PATH_DELIMITER_S GRIP_DIR PATH_DELIMITER_S))
						continue;

					filesNo++;
					canonizePath(fname, fileName);
					if (verbose >= 1)
						printProgress(indexer, filesNo, fileName);

					indexer.indexFile(fileName);

					if (indexer.size() >= chunkSize)
					{
						if (verbose >= 1)
						{
							reprint("writing chunks to database...");
							lastTime = steady_clock::now();
						}

						indexer.write();
						chunksNo++;
					}
				}
				catch (const Error &ex)
				{
					printFileError(ex, fname);
				}
			}
		}
		catch (const Error &ex)
		{
			printGenericError(ex);
			result = 2;
		}


		if (verbose >= 1)
		{
			reprint("sorting chunks database...");
		}

		files.close();
		indexer.sortDatabase();
		chunksNo++;

		if (verbose >= 1)
		{
			auto now = steady_clock::now();
			float duration = duration_cast<milliseconds>(now - startTime).count();
			float bytesSec = (float)indexer.filesTotalSize() * 1000.f / duration;
			float filesSec = (float)indexer.filesNo() * 1000.f / duration;

			reprint("done");

			println(" - files:    indexed %zu (%s), skipped %zu, total %zu",
					indexer.filesNo(),
					humanReadableSize(indexer.filesTotalSize()).c_str(),
					(filesNo - indexer.filesNo()),
					filesNo);

			println(" - speed:    %.1f files/sec, %s/sec",
					filesSec,
					humanReadableSize(bytesSec).c_str());

			println(" - time:     %.3f sec",
					duration / 1000.f);

			println(" - database: %s in %lu %s",
					humanReadableSize(indexer.chunksSize()).c_str(),
					chunksNo,
					chunksNo == 1 ? "chunk" : "chunks (merged to 1)");
		}
	}
	catch (const Error &ex)
	{
		printGenericError(ex);
		result = 1;
	}
	catch (const exception &ex)
	{
		printGenericError(FuncError(ex.what()));
		result = 1;
	}

	return result;
}

void printProgress(const Indexer &indexer, unsigned long chunksNo, const string &fileName)
{
	auto now = steady_clock::now();

	if (duration_cast<milliseconds>(now - lastTime) > milliseconds(1000))
	{
		float duration = duration_cast<milliseconds>(now - startTime).count();
		float speed = (float) indexer.filesNo() * 1000.f / duration;

		reprint("indexing file %lu+ (%.0f files/sec): %s",
				chunksNo, speed, fileName.c_str());

		lastTime = now;
	}
}

void printFileError(const Error &err, const char *fname)
{
	if (!supressErrors)
	{
		reprint("%s: %s; %s", fname, err.what(), err.get("msg").c_str());
		printnl();
	}
}

void printGenericError(const Error &err)
{
	println("error: %s", err.what());
	for (auto tag : err.tags)
		println("\t%s: %s", tag.first.c_str(), tag.second.c_str());
}

void usage(const char *name)
{
	printf("Usage: %s [OPTIONS] [LIST]\n"
	"Generate index for grip\n"
	"\n"
	"Options:\n"
	"  -u, --update              update existing index (reindex file)\n"
	"      --chunk-size=SIZE     set chunks size (in MB)\n"
	"  -v, --verbose[=LEVEL]     be verbose (repeat to increase)\n"
	"  -q, --quiet, --silent     be quiet\n"
	"  -s, --no-messages         suppress error messages\n"
	"  -h, --help                display this help and exit\n"
	"\n"
	"LIST is file containing list of files to index, one per line.\n"
	"With no LIST, standard input will be read instead\n"
	"Example: find -type f -and -size -128k | gripgen\n",
	name);
}

void version(const char *name)
{
	printf("%s (grip indexer) " VERSION_STR "\n"
	"Copyright (C) 2016 Free Software Foundation, Inc.\n"
	"License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
	"This is free software: you are free to change and redistribute it.\n"
	"There is NO WARRANTY, to the extent permitted by law.\n"
	"\n"
	"Written by Mike Szymaniak, http://sc0ty.pl\n",
	name);
}

