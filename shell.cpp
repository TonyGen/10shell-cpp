/* Currently, this implementation only support directives and statements. declarations and definitions are not allowed, therefore, for example, you can not assign a value to a variable name to be used in the next command */

#include "shell.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <iostream>

using namespace std;

/** Remove first occurrence of given item from list if present */
template <class A> static void remove (vector<A> &list, A item) {
	for (typename vector<A>::iterator it = list.begin(); it != list.end(); ++it)
		if (*it == item) {list.erase (it); return;}
}

/** Print current context to given stream */
void shell::showContext (compile::LinkContext &ctx, ostream &out) {
	for (unsigned i = 0; i < ctx.libPaths.size(); i++) out << "#librarypath " << ctx.libPaths[i] << "\n";
	for (unsigned i = 0; i < ctx.libNames.size(); i++) out << "#library " << ctx.libNames[i] << "\n";
	for (unsigned i = 0; i < ctx.includePaths.size(); i++) out << "#includepath " << ctx.includePaths[i] << "\n";
	for (unsigned i = 0; i < ctx.headers.size(); i++) out << ctx.headers[i] << "\n";
}

/** A Command is a C++ Directive or Statement */
class Command {
public:
	string command;
	Command (string command) : command(command) {}
	virtual void execute (compile::LinkContext &) = 0;
	virtual void drop (compile::LinkContext &) = 0;
};
/** A C++ preprocessor directive like #include or using, or a shell directive like #library.
 * A directive starts with # except for 'using' directive */
class Directive : public Command {
public:
	Directive (string command) : Command(command) {}
	void execute (compile::LinkContext &);
	void drop (compile::LinkContext &);
	/** first word with # removed */
	string opName () {return (command[0] == '#')
		? command.substr (1, command.find(' ') - 1)
		: command.substr (0, command.find(' '));}
	/** rest of line after opName */
	string argLine () {return command.substr (command.find(' ') + 1);}
};
/** Anything that does not start with # is interpreted as a C++ statement(s). The text is simply wrapped in a dummy function of type void() and executed. */
class Statement : public Command {
public:
	Statement (string command) : Command(command) {}
	void execute (compile::LinkContext &);
	void drop (compile::LinkContext &);
};
/** A C++ declaration declares a type or declares that a variable or function exists in some library but does not define it.
 * A variable declaration always begins with 'extern'.
 * A function declaration does not have a body and ends with ';'.
 * A class/struct declaration always begins with 'class'/'struct'.
 * A type synonym declaration always begins with 'typedef'. */
/** A C++ definition creates a function or variable. The variable may also be initialized.
 * A function definition has a body starting with '{' and ending with '}'.
 * A variable definition does not begin with 'extern', ends with ';' and may be initialized with '('...')' or '='.
 * The only way to tell the difference between a variable initialized with '('...')' and a function declaration is the function arguments will have types while the variable initialization will not. To make it worse the function argument types may elide the variable name so it is a single word just like a variable initialization argument. In this case we find out if the single arg is a variable or type by compiling a dummy function with the word in the type position. If it compiles then we conclude it is a type, otherwise a variable. */

typedef boost::shared_ptr<Command> Command_;

/** Detect type of C++ command. Throw BadCommand if can't detect type of Command */
static Command_ parseCommand (string command) {
	if (command[0] == '#') return Command_ (new Directive (command));
	if (command.compare (0, 6, "using ") == 0) return Command_ (new Directive (command));
	return Command_ (new Statement (command));
}

/** Execute command. Throw BadCommand if not understood (by compiler, linker, or this shell). */
void shell::execute (compile::LinkContext &ctx, string command) {
	if (command.empty()) return;
	parseCommand (command) -> execute (ctx);
}

void Directive::execute (compile::LinkContext &ctx) {
	string name = opName();
	if (name == "drop") parseCommand (argLine()) -> drop (ctx);
	else if (name == "dropall") ctx.clearAll();
	else if (name == "librarypath") ctx.libPaths.push_back (argLine());
	else if (name == "library") ctx.libNames.push_back (argLine());
	else if (name == "includepath") ctx.includePaths.push_back (argLine());
	else if (name == "context") {shell::showContext (ctx, cout); cout.flush();}
	else ctx.headers.push_back (command);
}
void Statement::execute (compile::LinkContext &ctx) {
	compile::exec (ctx, command);
}

void Directive::drop (compile::LinkContext &ctx) {
	string name = opName();
	if (name == "drop") throw shell::BadCommand ("can't drop", command);
	else if (name == "dropall") throw shell::BadCommand ("can't drop", command);
	else if (name == "librarypath") remove (ctx.libPaths, argLine());
	else if (name == "library") remove (ctx.libNames, argLine());
	else if (name == "includepath") remove (ctx.includePaths, argLine());
	else if (name == "context") throw shell::BadCommand ("can't drop", command);
	else remove (ctx.headers, command);
}
void Statement::drop (compile::LinkContext &ctx) {
	throw shell::BadCommand ("can't drop statement", command);
}
