# gdbfile-64: Emulated N64 file system access via GDB

*Allows a Nintendo 64 ROM running in the [**ares**](https://ares-emu.net/) emulator to read and write files of the host file system.*

Homebrew ROMs made with the [**libdragon** SDK](https://libdragon.dev/) can already use a simulated DragonFS (DFS) filesystem.
Files are bundled in the ROM at build time and can be read via `fopen` and other common C standard library functions.
But what if you'd like to reload a file when the program is running? How about writing to one?

This library, **gdbfile-64**, allows an N64 ROM running in the **ares** emulator to read and write files at runtime.
It's based on breakpoints set on specific hooked functions, calls to which are intercepted and handled by a separate GDB Python script.
Only files in a specific directory are accessible.

So far this is **Linux-only**. Windows support may be possible.

## Building and running

You need to set the `N64_INST` environment variable before building.
Then build `example.n64` with

```
make
```

To test that everything works, first start the file hook.
As arguments you give

- the directory the ROM may access, and
- a path to an ELF file for debug symbols.

```
./run_file_hook.sh testfiles build/example.elf
```

Then launch **ares** with [the GDB support and homebrew mode turned on](https://github.com/DragonMinded/libdragon/wiki/Debugging-via-gdb):

```
ares example.n64
```

Now when you edit `testfiles/hello.txt` you should see the drawn text update.
Also a `testfiles/output.txt` file should've appeared. To verify this, run:

```
$ cat testfiles/output.txt 
It works!⏎                                      
```

## Credits and license

This project is released into the public domain. See `LICENSE` for details.
Thanks to Max Bebök for the original GDB hook example.
