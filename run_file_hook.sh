#!/bin/bash
# Starts the file hook server in the given root directory.
# Meant to be used with the ares' emulators GDB server.
# See file_hook.py for details.
#
# Usage: ./run_file_hook.sh testfiles
set -o nounset
GDB=gdb-multiarch
ELF=build/example.elf # Your ROM's .elf file for symbols
while true; do
    GDB_FILE_HOOK_DIR=$1 \
    $GDB --batch -ex "target remote localhost:9123" -ex "file ${ELF}" -x file_hook.py -ex "continue"
    sleep 0.2
done

