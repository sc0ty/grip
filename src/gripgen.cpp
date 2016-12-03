#include "indexer.h"
#include "dir.h"
#include "fileline.h"
#include "config.h"
#include "print.h"
#include "error.h"
#include <chrono>
#include <cstring>
#include <cstdio>
#include <unistd.h>

using namespace std;
using namespace std::chrono;


static void printProgress();
static void usage(const char *name);


static string fileName;
static unsigned fileNo = 0;
static steady_clock::time_point startTime, lastTime;


int main(int argc, char * const argv[])
{
	try
	{
		Indexer indexer;
		FileLineReader files;
		int verbose = 1;
		size_t chunkSize = 64 * 1024 * 1024;

		int opt;
		while ((opt = getopt(argc, argv, "s:vqh")) != -1)
		{
			switch (opt)
			{
				case 's':
					chunkSize = atol(optarg) * 1024 * 1024;
					break;

				case 'v':
					verbose++;
					break;

				case 'q':
					verbose = 0;
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
				println("reading list from file %s", optarg);
			files.open(optarg);
		}
		else
		{
			if (verbose >= 2)
				println("reading list from standard input");
			files.open(stdin);
		}

		const char *fname = NULL;
		unsigned long chunksNo = 0;
		int id = optind;

		if (verbose >= 2)
		{
			println("max chunk size: %lu MB",
					(unsigned long)chunkSize / (1024*1024));
		}


		indexer.open();
		startTime = steady_clock::now();

		if (verbose)
			print("indexing...");

		while (true)
		{
			try
			{
				if (id < argc)
					fname = argv[id++];
				else if (files.isOpen())
					fname = files.readLine(false);
				else
					break;

				if (fname == NULL)
					break;

				if (strncmp(fname, GRIP_DIR, sizeof(GRIP_DIR) - 1) == 0)
					continue;

				if (strstr(fname, "/" GRIP_DIR "/"))
					continue;

				canonizePath(fname, fileName);
				fileNo++;

				if (verbose)
					printProgress();

				indexer.indexFile(fileName);

				if (indexer.size() >= chunkSize)
				{
					if (verbose)
						reprint("writing chunks to database...");

					indexer.write();
					chunksNo++;
				}
			}
			catch (const Error &ex)
			{
				reprint("%s: %s; %s", fname, ex.what(), ex.get("msg").c_str());
				printnl();
				lastTime = steady_clock::time_point();
			}
		}

		if (verbose)
			reprint("sorting chunks database...");

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
					(unsigned long)(fileNo - indexer.filesNo()),
					(unsigned long)fileNo);

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

void printProgress()
{
	auto now = steady_clock::now();

	if (duration_cast<milliseconds>(now - lastTime) > milliseconds(1000))
	{
		float duration = duration_cast<milliseconds>(now - startTime).count();
		float speed = (float)fileNo * 1000.f / duration;

		reprint("indexing file %u (%.0f files/sec): %s",
				fileNo, speed, fileName.c_str());

		lastTime = now;
	}
}

void usage(const char *name)
{
	printf("Usage: %s [OPTIONS] [LIST]\n"
	"Generate index for grip\n"
	"\n"
	"Options:\n"
	"  -v      be verbose (repeat to increase)\n"
	"  -q      be quiet\n"
	"  -h      print this help\n"
	"\n"
	"LIST is file containing list of files to index, one per line.\n"
	"With no LIST, standard input will be read instead\n"
	"Example: find -type f -and -size -128k | gripgen\n",
	name);
}

