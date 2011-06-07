Install dependent library first

- [10util](https://github.com/TonyGen/10util-cpp)
- readline

Download and remove '-ccp' suffix

	git clone git://github.com/TonyGen/10shell-cpp.git 10shell
	cd remote

Build library `lib10shell.so` and program `10shell`

	scons

Install library in `/usr/local/lib`, header files in `/usr/local/include/10shell`, and program in `/usr/local/bin`

	sudo scons install
