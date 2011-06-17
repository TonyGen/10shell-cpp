Execute C++ statements in an interactive shell. Each statement is compiled, dynamically loaded, and executed. Each statement takes a second or two to compile, but is tolerable.

To give context (libraries and headers) to each C++ statement, the shell also accepts directives. A directive is a 'using namespace ...' directive, a pre-processor directive like '#include ...', or one of the special shell directive, below. The directives are remembered and used by every future statement. The context is even remembered across sessions in a file local to the working directory.
The special shell directive are:

- '#librarypath Path' where Path is supplied to the -L option of the compiler
- '#library Libname' where Libname is supplied to the -l option of the compiler
- '#includepath Path' where Path is supplied to the -I option of the compiler
- '#context' lists all previously entered directives
- '#drop Directive' where Directive is a previously entered directive. Drop removes the directive for memory, ie. undoes the directive
- '#dropall' removes all previous directives from context memory

Note, statements are not remembered, so a statement can not use a variable assigned by a previous statement. This was done for simplicity of implementation and because I don't think you loose much expressive power as exemplified by normal Bash shell usage. Most people do not create temporary variables in Bash to be used later. You can create temporary variables for the same statement, i.e. a statement can be a sequence of statements.

### Installing

Install dependent library first

- [10util](https://github.com/TonyGen/10util-cpp)
- boost-program-options-dev
- readline

Download and remove '-ccp' suffix

	git clone git://github.com/TonyGen/10shell-cpp.git 10shell
	cd remote

Build library `lib10shell.so` and program `10shell`

	scons

Install library in `/usr/local/lib`, header files in `/usr/local/include/10shell`, and program in `/usr/local/bin`

	sudo scons install
