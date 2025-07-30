# gdbfile-64: Emulated N64 file system access via GDB

*Allows a Nintendo 64 ROM running in the [**ares**](https://ares-emu.net/) emulator to read and write files of the host file system.*

This library, **gdbfile-64**, allows an N64 ROM running in the **ares** emulator to read and write files at runtime.
It's based on breakpoints set on specific hooked functions, calls to which are intercepted and handled by a separate GDB Python script.
Only files in a specific directory are accessible.

This is how using the library looks like:

```c
// Try to read a file
uint8_t contents[100];
int size = gdbfile_read("hello.txt", sizeof(contents)-1, contents);

if (size > -1) {
    // GDB just copied bytes to memory so terminate the string
    contents[size] = '\0';
    // Print the result
    graphics_draw_text(disp, 16, 32, (const char*)contents);
    // Write back an answer just for show
    gdbfile_write_string("output.txt", "It works!");
}
```

So far this is **Linux-only**. Windows support may be possible.
        

## Building and running

You need the [**libdragon** SDK](https://libdragon.dev/) installed and its  `N64_INST` environment variable set before building.
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

You should see the contents of `testfiles/hello.txt` appear on screen:

<img width="646" height="561" alt="image" src="https://github.com/user-attachments/assets/6cdd2790-99f4-4bfa-8a5a-8859755e4111" />

Now when you edit the file, you should see the drawn text update:

<img width="644" height="561" alt="image" src="https://github.com/user-attachments/assets/48ed7429-782a-4d61-8452-83eed70051ce" />

Also a `testfiles/output.txt` file should've appeared. To verify this, run:

```
$ cat testfiles/output.txt 
It works!⏎                                      
```

## Relation to libdragon's DFS

Homebrew ROMs made with the **libdragon** can already use a simulated DragonFS (DFS) filesystem.
Files are bundled in the ROM at build time and can be read via `fopen` and other common C standard library functions.

This library doesn't interoperate with DFS at all. But you can set the `filesystem/` library as the target directory, which allows you to reload files that will get eventually included in the ROM on the next rebuild. Unfortunately loading and reloading must be done via the respective DFS and gdbfile functions, leading to duplicated code.


## Credits and license

This project is released into the public domain. See `LICENSE` for details.
Thanks to Max Bebök for the original GDB hook example.
