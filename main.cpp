/* Accept C++ commands from stdin and execute them printing non-empty results to stdout */

#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <execinfo.h> // backtrace
#include <iostream>

using namespace std;

/** Execute C++ commands from stdin one at a time until EOF. Print any results to stdout, and errors to stderr */
static void interactionLoop () {
	compile::LinkContext myShell;
	for (;;) {
		char* line = readline ("> ");
		if (! line) break; // EOF
		add_history (line);
		string command (line);
		delete line;
		try {
			shell::execute (myShell, command);
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
	interactionLoop();
	return 0;
}
