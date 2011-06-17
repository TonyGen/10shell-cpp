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
#include <10util/except.h>
#include <boost/bind.hpp>
#include <10util/thread.h>
#include <cstdlib> // system
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

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

static void executeCmd (compile::LinkContext* myShell, string command) {
	try {
		shell::execute (*myShell, command);
	} catch (exception &e) {
		cerr << typeName(e) << ": " << e.what() << endl;
	}
}

static string metadir;

static bool nocatch;

static thread::Thread currentCommand = thread::Thread(); // empty

/** Execute C++ commands from stdin one at a time until EOF. Print any results to stdout, and errors to stderr */
static void interactionLoop () {
	using_history();
	read_history ((metadir + "/history").c_str());
	compile::LinkContext myShell;
	loadContext (myShell, metadir + "/context");
	shell::showContext (myShell, cout);
	for (;;) {
		char* line = readline ("> ");
		if (! line) break; // EOF
		add_history (line);
		string command (line);
		delete line;
		if (nocatch)
			shell::execute (myShell, command);
		else {
			boost::function0<void> act = boost::bind (executeCmd, &myShell, command);
			currentCommand = thread::fork (act);
			currentCommand->join();
			currentCommand.reset();
		}
	}
	saveContext (myShell, metadir + "/context");
	write_history ((metadir + "/history").c_str());
}

void ensureDir (string dir) {
	system (("mkdir -p " + dir).c_str());
}

void interrupt_handler (int sig) {
	if (currentCommand) currentCommand->interrupt();
}

void segfault_handler (int sig) {
	except::raise (runtime_error ("SegFault"));
}

void get_program_options (int argc, char* argv[]) {
	po::options_description desc (
		"\nExecute C++ statements in an interactive shell. Each statement is compiled, dynamically loaded, and executed. "
		"Each statement takes a second or two to compile, but is tolerable.\n\n"
		"To give context (libraries and headers) to each C++ statement, the shell also accepts directives. "
		"A directive is a 'using namespace ...' directive, a pre-processor directive like '#include ...', "
		"or one of the special shell directive, below. The directives are remembered and used by every future statement. "
		"The context is even remembered across sessions in a file local to the working directory.\n"
		"The special shell directive are:\n\n"
		"'#librarypath Path' where Path is supplied to the -L option of the compiler\n"
		"'#library Libname' where Libname is supplied to the -l option of the compiler\n"
		"'#includepath Path' where Path is supplied to the -I option of the compiler\n"
		"'#context' lists all previously entered directives\n"
		"'#drop Directive' where Directive is a previously entered directive. Drop removes the directive for memory, ie. undoes the directive\n"
		"'#dropall' removes all previous directives from context memory\n\n"
		"Note, statements are not remembered, so a statement can not use a variable assigned by a previous statement. "
		"This was done for simplicity of implementation and because I don't think you loose much expressive power as "
		"exemplified by normal Bash shell usage. Most of the time you don't create temporary variables in Bash shell to be used later. "
		"You can create temporary variables for the same statement, i.e. a statement can be a sequence of "
		"statements.\n\n"
		"Allowed options"
	);
	desc.add_options()
		("help,h", "This help message")
		("nocatch", "On error, abort shell. Useful when piping commands to stdin and don't want to silently skip errors")
		("metadir", po::value<string>(&metadir)->default_value("."+string(argv[0])), "Directory where context and history file is found");
	po::positional_options_description p;
	p.add ("metadir", -1);
	po::variables_map vm;
	po::store (po::command_line_parser (argc, argv) .options (desc) .positional (p) .run(), vm);
	po::notify (vm);
	nocatch = vm.count ("nocatch");
	if (vm.count ("help")) {cout << desc << endl; exit(0);}
}

int main (int argc, char* argv[]) {
	get_program_options (argc, argv);
	signal (SIGSEGV, segfault_handler);
	if (!nocatch) signal (SIGINT, interrupt_handler);
	ensureDir (metadir);
	interactionLoop ();
	return 0;
}
