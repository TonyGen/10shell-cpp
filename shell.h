/* Execute C++ commands one at a time.
 * A command can be any C++ preprocessor-directive, declaration, definition, or statement.
 * Implementation:
 * Each definition is compiled to its own dynamic library and loaded. The OS initializes the definition when loaded.
 * Each statement is compiled inside a dummy function, loaded, and executed. A statement may contain multiple statements each terminated by ';'.
 * Each directive, declaration, and definition prototype is remembered and included when compiling future definitions and statements.
 * Each library (see below) and definition is remembered and linked with future definitions and statements.
 * New shell directives are defined to specify dependent libraries and to undo previous directives, declaration, and definitions. The new directive are:
 *   #librarypath <path, given to -L compiler option>
 *   #library <lib name without prefix or suffix, given to -l compiler option>
 *   #includepath <path, given to -I compiler option>
 *   #drop X. remove any previous directive, declaration, or definition X. For example:
 *     #drop #librarypath <path, removed from -L compiler option>
 *     #drop #library <lib name, removed from -l compiler option>
 *     #drop #includepath <path, removed from -I compiler option>
 *     #drop #include <header file>
 *     #drop int x;
 *   #dropall. remove all previous directives, declarations and definitions. Equivalent to restarting program.
 *   #context. show current context built up from previous directives, declarations, and definitions.
 */

#pragma once

#include <sstream>
#include <stdexcept>
#include <10util/compile.h>

namespace shell {

/** Execute command. Throw BadCommand if command not understood (by compiler, linker, or this shell). */
void execute (compile::LinkContext &ctx, std::string command);

/** Thrown when command not understood by compiler, linker, or this shell */
class BadCommand : public std::exception {
public:
	std::string error;
	std::string command;
	BadCommand (std::string error, std::string command) : error(error), command(command) {}
	~BadCommand () throw () {}
	virtual const char* what() const throw() {
		std::stringstream ss;
		ss << error << ": " << command;
		return ss.str().c_str();
	}
};

}
