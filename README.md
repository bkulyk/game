# Game

This is going to be a game. Likely developed in C with SGDK/Mars. I'm still figuring it out

## Building

```sh
# build with the mars toolchain, launch with emulator
task dev
```

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