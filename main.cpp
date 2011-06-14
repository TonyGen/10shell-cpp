/* Accept C++ commands from stdin and execute them printing non-empty results to stdout.
 * Persistent context and history or stored in .10shell directory in working directory */

#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <10util/stacktrace.h>
#include <10util/util.h>
#include <stdexcept>
#include <boost/bind.hpp>
#include <10util/thread.h>

using namespace std;

static void loadContext (compile::LinkContext &shell, string contextFilename) {
	ifstream contextFile (contextFilename.c_str());
	string line;
	while (contextFile.good()) {
		getline (contextFile, line);
		shell::execute (shell, line);
	}
}

static void saveContext (compile::LinkContext &shell, string contextFilename) {
	ofstream contextFile (contextFilename.c_str());
	shell::showContext (shell, contextFile);
}

/** Execute command */
static void executeCmd (compile::LinkContext* myShell, string command) {
	try {
		shell::execute (*myShell, command);
	} catch (exception &e) {
		cerr << typeName(e) << ": " << e.what() << endl;
	}
}

static thread::Thread currentCommand = thread::Thread(); // empty

/** Execute C++ commands from stdin one at a time until EOF. Print any results to stdout, and errors to stderr */
static void interactionLoop (string metaDir) {
	using_history();
	read_history ((metaDir + "/history").c_str());
	compile::LinkContext myShell;
	loadContext (myShell, metaDir + "/context");
	shell::showContext (myShell, cout);
	for (;;) {
		char* line = readline ("> ");
		if (! line) break; // EOF
		add_history (line);
		string command (line);
		delete line;
		boost::function0<void> act = boost::bind (executeCmd, &myShell, command);
		currentCommand = thread::fork (act);
		currentCommand->join();
		currentCommand.reset();
	}
	saveContext (myShell, metaDir + "/context");
	write_history ((metaDir + "/history").c_str());
}

void ensureDir (string dir) {
	//TODO
}

void interrupt_handler (int sig) {
	if (currentCommand) currentCommand->interrupt();
}

void segfault_handler (int sig) {
	print_stacktrace ();
	interrupt_handler (sig);
}

int main (int argc, char* argv[]) {
	signal (SIGSEGV, segfault_handler);
	signal (SIGINT, interrupt_handler);
	string metaDir = argc > 1 ? argv[1] : "." + string (argv[0]);
	ensureDir (metaDir);
	interactionLoop (metaDir);
	return 0;
}
