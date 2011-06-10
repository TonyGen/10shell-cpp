/* Accept C++ commands from stdin and execute them printing non-empty results to stdout */

#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <execinfo.h> // backtrace
#include <iostream>
#include <fstream>

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

/** Execute C++ commands from stdin one at a time until EOF. Print any results to stdout, and errors to stderr */
static void interactionLoop (string contextFilename) {
	compile::LinkContext myShell;
	loadContext (myShell, contextFilename);
	for (;;) {
		char* line = readline ("> ");
		if (! line) break; // EOF
		add_history (line);
		string command (line);
		delete line;
		try {
			shell::execute (myShell, command);
			saveContext (myShell, contextFilename);
		} catch (exception &e) {
			cerr << /*typeName(e) << ": " <<*/ e.what() << endl;
			void *array[30];
			size_t size;
			size = backtrace (array, 30);  // get void*'s for all entries on the stack
			backtrace_symbols_fd (array, size, 2);  // print out all the frames to stderr
		}
	}
}

int main (int argc, char* argv[]) {
	string contextFile = argc > 1 ? argv[1] : "." + string (argv[0]);
	interactionLoop (contextFile);
	return 0;
}
