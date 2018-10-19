#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#include <vector>
#include <set>
#include <string>

#define main gripMain
#include "grip.cpp"
#undef main

using namespace std;


enum PipeDirection
{
	READ_FD  = 0,
	WRITE_FD = 1
};


class Command
{
	public:
		Command() : pid(0)
		{
			pipe(pipefd);
		}

		void dumpOutput()
		{
			pthread_create(&thread, NULL, Command::dumpOutputJobTrampoline, this);
		}

		void waitForDump()
		{
			pthread_join(thread, NULL);
		}

		int waitForChild()
		{
			int status = 0;
			waitpid(pid, &status, 0);
			return WEXITSTATUS(status);
		}

		const set<string> &getLines() const
		{
			return lines;
		}

		void printLines(FILE *fp = stdout) const
		{
			for (const string &line : lines)
				fprintf(fp, "%s\n", line.c_str());
		}

		void terminate()
		{
			kill(pid, SIGKILL);
		}

	private:
		static void *dumpOutputJobTrampoline(void *obj)
		{
			Command *cmd = (Command*) obj;
			cmd->dumpOutputJob();
			return NULL;
		}

		void dumpOutputJob()
		{
			/*char buf[1024];
			ssize_t res;

			do
			{
				res = read(pipefd[READ_FD], buf, sizeof(buf)-sizeof(char));
				if (res == -1)
				{
					if ((errno != EINTR) && (errno != EAGAIN))
						exit(-2);
				}
				else if (res > 0)
				{
					lines.append(buf, res);
				}
			}
			while (res != 0);*/

			FILE *fp = fdopen(pipefd[READ_FD], "r");
			char *line = NULL;
			size_t len = 0;
			ssize_t read;

			while ((read = getline(&line, &len, fp)) != -1)
			{
				if (read > 0)
				{
					if (line[read-1] == '\n')
						line[read-1] = '\0';

					lines.insert(line);
				}
			}
		}

	protected:
		int pipefd[2];
		pid_t pid;
		set<string> lines;
		pthread_t thread;
};


class ExtCommand : public Command
{
	public:
		void run(const vector<char*> &args)
		{
			fflush(stdout);
			pid = fork();

			if (pid == -1)
			{
				exit(-1);
			}
			else if (pid == 0)
			{
				// child process
				dup2(pipefd[WRITE_FD], STDOUT_FILENO);
				dup2(pipefd[WRITE_FD], STDERR_FILENO);
				close(pipefd[READ_FD]);
				execvp(args[0], args.data());
			}
			else
			{
				// parent process
				close(pipefd[WRITE_FD]);
			}
		}
};


class GripCommand : public Command
{
	public:
		GripCommand()
		{
			stdout_bk = dup(STDOUT_FILENO);
			dup2(pipefd[WRITE_FD], STDOUT_FILENO);
		}

		int run(const vector<char*> &args)
		{
			int res = gripMain(args.size()-1, args.data());
			fflush(stdout);

			close(pipefd[WRITE_FD]);
			dup2(stdout_bk, STDOUT_FILENO);
			return res;
		}

	private:
		int stdout_bk;
};


int main(int argc, char *argv[])
{
	vector<char*> gripArgs;
	vector<char*> extArgs;
	vector<char*> *ptrArgs = NULL;

	bool abortOnDiff = false;

	for (int i = 1; i < argc; i++)
	{
		char *opt = argv[i];

		if (strcmp(opt, "--grip-cmd") == 0)
		{
			ptrArgs = &gripArgs;
		}
		else if (strcmp(opt, "--ext-cmd") == 0)
		{
			ptrArgs = &extArgs;
		}
		else if (ptrArgs)
		{
			ptrArgs->push_back(opt);
		}
		else if (strcmp(opt, "--abort") == 0)
		{
			abortOnDiff = true;
		}
		else
		{
			fprintf(stderr, "invalid argument \"%s\"\n", opt);
			return -1;
		}
	}

	gripArgs.push_back(NULL);
	extArgs.push_back(NULL);

	ExtCommand ext;
	ext.run(extArgs);
	ext.dumpOutput();

	GripCommand grip;
	grip.dumpOutput();
	int gripRes = grip.run(gripArgs);
	grip.waitForDump();

	if (gripRes == 0 || gripRes == 1)
	{
		int extRes = ext.waitForChild();
		ext.waitForDump();

		if (gripRes != extRes || grip.getLines() != ext.getLines())
		{
			if (abortOnDiff)
				abort();

			return 1;
		}
		return 0;
	}
	else if (gripRes == 3)
	{
		// invalid query
		ext.terminate();
		ext.waitForChild();
		ext.waitForDump();
		return 0;
	}
	else
	{
		// error occurred
		ext.waitForChild();
		ext.waitForDump();

		if (abortOnDiff)
			abort();

		return gripRes;
	}

	return 0;
}
