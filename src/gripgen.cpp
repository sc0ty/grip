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

#ifdef _POSIX_C_SOURCE
#include <unistd.h>
#include <getopt.h>
#else
#include "external/getopt.h"
#include "external/getopt.c"
#include "external/getopt1.c"
#endif

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
	{NULL, 0, NULL, 0}
};

static char const SHORTOPTS[] = "hqsuv";


static void printProgress(const Indexer &indexer);
static void usage(const char *name);


static string fileName;
static steady_clock::time_point startTime, lastTime;


int main(int argc, char * const argv[])
{
	try
	{
		Indexer indexer;
		size_t chunkSize = 64 * 1024 * 1024;

		FileLineReader files;
		unsigned allFilesNo = 0;

		int verbose = 1;
		bool supressErrors = false;
		bool updateIndex = false;

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

				case 'h':
				default:
					usage(argv[0]);
					return 0;
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

		const char *fname = NULL;
		unsigned long chunksNo = 0;

		if (verbose >= 2)
		{
			println("max chunk size: %lu MB",
					(unsigned long)chunkSize / (1024*1024));
		}


		indexer.open();
		startTime = lastTime = steady_clock::now();

		if (verbose >= 1)
			print("indexing...");

		while (true)
		{
			try
			{
				fname = files.readLine(false);
				if (fname == NULL)
					break;

				if (strncmp(fname, GRIP_DIR, sizeof(GRIP_DIR) - 1) == 0)
					continue;

				if (strstr(fname, "/" GRIP_DIR "/"))
					continue;

				canonizePath(fname, fileName);
				allFilesNo++;

				if (verbose >= 1)
					printProgress(indexer);

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
				if (!supressErrors)
				{
					reprint("%s: %s; %s", fname, ex.what(), ex.get("msg").c_str());
					printnl();
					lastTime = steady_clock::time_point();
				}
			}
		}

		if (verbose >= 1)
			reprint("sorting chunks database...");

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

			println(" - files:    indexed %lu (%s), skipped %lu, total %lu",
					(unsigned long)(indexer.filesNo()),
					humanReadableSize(indexer.filesTotalSize()).c_str(),
					(unsigned long)(allFilesNo - indexer.filesNo()),
					(unsigned long)allFilesNo);

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
		println("error: %s", ex.what());
		for (auto tag : ex.tags)
			println("\t%s: %s", tag.first.c_str(), tag.second.c_str());

		return 1;
	}

	return 0;
}

void printProgress(const Indexer &indexer)
{
	auto now = steady_clock::now();

	if (duration_cast<milliseconds>(now - lastTime) > milliseconds(1000))
	{
		float duration = duration_cast<milliseconds>(now - startTime).count();
		unsigned filesNo = indexer.filesNo();
		float speed = (float)filesNo * 1000.f / duration;

		reprint("indexing file %u (%.0f files/sec): %s",
				filesNo, speed, fileName.c_str());

		lastTime = now;
	}
}

void usage(const char *name)
{
	printf("Usage: %s [OPTIONS] [LIST]\n"
	"Generate index for grip\n"
	"Author: Mike Szymaniak, http://sc0ty.pl\n"
	"\n"
	"Options:\n"
	"  -u, --update              update existing index (reindex file)\n"
	"      --chunk-size=SIZE     set chunks size (in MB)\n"
	"  -v, --verbose[=LEVEL]     be verbose (repeat to increase)\n"
	"  -q, --quiet, --silent     be quiet\n"
	"  -s, --no-messages         supress error messages\n"
	"  -h, --help                display this help and exit\n"
	"\n"
	"LIST is file containing list of files to index, one per line.\n"
	"With no LIST, standard input will be read instead\n"
	"Example: find -type f -and -size -128k | gripgen\n",
	name);
}

