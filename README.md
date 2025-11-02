# Game

This is going to be a game. Likely developed in C with SGDK/Mars. I'm still figuring it out

## Building

```sh
# build with the mars toolchain, launch with emulator
task dev
```

### Dependencies

To build on a mac and avoid docker I'm using [marsdev](https://github.com/andwn/marsdev/), I found it tricky to setup.

I had to make some tweaks to use clang from homebrew instead of whatever comes with the Mac

```sh
export CC=/opt/homebrew/opt/llvm@16/bin/clang && export CXX=/opt/homebrew/opt/llvm@16/bin/clang++ && make all
```

but that didn't work for everything so in md-tools-flamewing, I had to make a manual change to point to `/opt/homebrew/opt/llvm@16/bin/clang++`

and mdtools-sik I had to

```makefile
CFLAGS := $(CFLAGS) -I/opt/homebrew/opt/libpng/include
LDFLAGS := $(LDFLAGS) -L/opt/homebrew/opt/libpng/lib
```

The version of SGDK was dated and the example were not working without remapping some functions, so I changed the SGDK_VER.

```makefile
# SGDK_VER ?= __UNSET__
SGDK_VER ?= v2.11
```

I'm sure there are more elegant ways to make these changes over editing files, but I was having trouble and in a rush.

## Emulators

I found 2 emulators to work for Mac [genesis plus] and [blastem].

### Genesis Plus

Seem to work well can open roms from cli like: `open -s "genesis plus" rom.bin`.

Genesis Plus doesn't seem to have much in the way of debugging.

### Blastem

I had to compile it myself to get it to work with my M4 laptop.

```sh
# get dependency, may require others that were already installed for me.
brew install glew
# Add missing CPU core files using the project's cpu_dsl.py script
python3 cpu_dsl.py -d call m68k.cpu > m68k.c
python3 cpu_dsl.py -d call z80.cpu > z80.c
python3 cpu_dsl.py -d call upd78k2.cpu > upd78k2.c
# compile
make
```

[blastem]: https://www.retrodev.com/blastem/
[genesis plus]: https://segaretro.org/Genesis_Plus