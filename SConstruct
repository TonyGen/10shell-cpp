libname = '10shell'

lib = SharedLibrary (libname, ['shell.cpp'],
	CCFLAGS = ['-g', '-rdynamic'],
	CPPPATH = ['.', '/opt/local/include'],
	LIBPATH = ['/opt/local/lib'],
	LIBS = ['10util'] )

prog = Program (libname, ['main.cpp'],
	CCFLAGS = ['-g', '-rdynamic'],
	CPPPATH = ['.', '/opt/local/include'],
	LIBPATH = ['.', '/opt/local/lib'],
	LIBS = [libname, '10util', 'boost_thread-mt', 'boost_program_options-mt', 'readline'] )

Alias ('install', '/usr/local')
Install ('/usr/local/lib', lib)
Install ('/usr/local/include/' + libname, Glob('*.h'))
Install ('/usr/local/bin', prog)
