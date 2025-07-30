# Emulated N64 file system access via GDB

*Allows an N64 ROM running in the **ares** emulator to detect changes and pull updated files from the host.**

This example is for the [homebrew **libdragon** SDK](https://libdragon.dev/) for the Nintendo 64.
You need to set the `N64_INST` environment variable before building.
Then to produce `example.n64` you do just

```
make
```

And to run it, first start the file hook:

```
./run_file_hook.sh testfiles
```

and then **ares** with the GDB support and homebrew mode turned on:

```
ares example.n64
```

Now when you edit `testfiles/hello.txt` you should see it updated in **ares**.