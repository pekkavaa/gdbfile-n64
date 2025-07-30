"""
Allows the inferior to access host's file system.
Reads and writes files in the given GDB_FILE_HOOK_DIR env var.
The paths are sanitized against parent traversal to prevent accidents,
but shouldn't be considered secure.

This requres support on the guest side, see gdb.cpp.

This script is meant to be executed right after connecting to a remote.
In case the script is started with old symbols and the hook functions aren't
found the, the script terminates GDB and lets run_file_hook.py restart itself.
"""

import sys
import os
import traceback
# import ctypes
import pathlib
from datetime import datetime

# The gdb module is automatically imported but it's nice to have here for IDE autocompletion.
import gdb

# A symbol has an address if its 'address' query doesn't throw.
# This is the only robust way I found to check for the existence of a symbol.
def check_if_symbol_has_address(name: str) -> bool:
  try:
    result = gdb.execute(f"info address {name}", to_string=True)
    # print("result", result)
    return True
  except gdb.error:
    return False

fileLastModifiedSymbol = 'GDB::fileLastModified'
writeFileSymbol = 'GDB::writeFile'
readFileSymbol = 'GDB::readFile'

any_function_found = False
any_function_found |= check_if_symbol_has_address(fileLastModifiedSymbol)
any_function_found |= check_if_symbol_has_address(writeFileSymbol)
any_function_found |= check_if_symbol_has_address(readFileSymbol)

# If we couldn't find any addresses the breakpoints probably (?) won't fire anyway
# so kill GDB and let it be restarted by the runner script with fresh symbols.
if not any_function_found:
  print("No function symbols found. Exiting.")
  gdb.execute("quit")


tmp_dir = os.environ.get('GDB_FILE_HOOK_DIR', None)
print(f"{tmp_dir=}")

assert tmp_dir, "Root dir environment variable wasn't set"


def sanitize_path(path):
    """
    Sanitize a path against directory traversals
    """
    # From: https://stackoverflow.com/a/66950540
    # - pretending to chroot to the current directory
    # - cancelling all redundant paths (/.. = /)
    # - making the path relative
    return os.path.relpath(os.path.normpath(os.path.join("/", path)), "/")


def map_path(path: str):
  return os.path.join(tmp_dir, sanitize_path(path))


def getReturnValues():
  frame = gdb.selected_frame()
  returnValues, _ =  gdb.lookup_symbol("returnValues", frame.block().static_block)
  return returnValues.value()

def assign(var, value):
  return gdb.execute(f"set {var}={value}", to_string=True)

earlier_modified_ms = 0


class BPFileLastModified(gdb.Breakpoint):
  def stop (self):
    global earlier_modified_ms

    frame = gdb.selected_frame()
    path = frame.read_var("path").string()
    real_path = map_path(path)

    returnValues, _ =  gdb.lookup_symbol("returnValues", frame.block().static_block)

    last_modified = returnValues.value()["gdbLastModified"]
    # gdb.execute(f"set returnValues.gdbLastModified=0", to_string=True)
    assign("returnValues.gdbLastModified", "0")

    try:
      last_modified_ms = int(pathlib.Path(real_path).stat().st_mtime_ns / 1e6)
    except OSError as e:
      print(e)
      return False


    timestamp_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    if last_modified_ms != earlier_modified_ms:
      print(f"[{timestamp_str}] {path} {last_modified_ms=}")
      earlier_modified_ms = last_modified_ms
    # print(f"{last_modified=}")
    # last_modified.assign(last_modified_ms)

    assign("returnValues.gdbLastModified", last_modified_ms)
    # gdb.execute(f"set returnValues.gdbLastModified={last_modified_ms}", to_string=True)
    # print(f"assigned {last_modified_ms} to {time}")
    # print("time value now:\n", time_value);


class BPWriteFile(gdb.Breakpoint):
  def stop (self):
    # assert ValueError("WriteFile not implemented (ctypes dep not working)")
    frame = gdb.selected_frame()
    path = frame.read_var("path").string()

    # print(f"save {path=}")

    # data_addr = str(ctypes.c_uint32(frame.read_var("data")).value)
    data_addr = frame.read_var("data")
    size = int(frame.read_var("dataSizeBytes"))

    # wrote_bytes = getReturnValues()["gdbWroteBytes"]
    # wrote_bytes.assign(-1) # Set a return value of -1 in case open() throws

    assign("returnValues.gdbWroteBytes", -1)

    data = gdb.selected_inferior().read_memory(data_addr, size)

    real_path = map_path(path)
    print(f"{path=} {data=}")
    print(f"{real_path=}")

    try:
      with open(real_path, "wb") as f:
        wrote = f.write(data)
    except OSError as e:
      print(e)
      return False

    assign("returnValues.gdbWroteBytes", wrote)
    # wrote_bytes.assign(wrote)
    print(f"Wrote {len(data)} bytes to {real_path}")
    return False


class BPReadFile(gdb.Breakpoint):
  def stop_impl(self):
    remote = gdb.selected_inferior()
    frame = gdb.selected_frame()
    path = frame.read_var("path").string()

    # print(f"read {path=}")

    max_size = int(frame.read_var("maxSize"))
    dest = frame.read_var("dest")
    # dest_addr: str = str(ctypes.c_uint32(dest).value)
    dest_addr = dest

    read_bytes = getReturnValues()["gdbReadBytes"]
    # read_bytes.assign(-1) # Set a return value of -1 in case open() throws
    assign("returnValues.gdbReadBytes", -1)

    real_path = map_path(path)

    try:
      with open(real_path, "rb") as f:
        raw_data = f.read()
    except FileNotFoundError as e:
      print(e)
      return False

    data = raw_data[:max_size]
    remote.write_memory(dest_addr, data)

    # read_bytes.assign(len(data))
    assign("returnValues.gdbReadBytes", len(data))

    # print(f"Read {len(data)} bytes from '{path}' to {hex(ctypes.c_uint32(dest).value)}")
    print(f"Read {len(data)} bytes from '{path}' to {hex(int(dest))}")

    return False


  def stop(self):
    try:
        self.stop_impl()
    except Exception:
        print(traceback.print_exc())

      
# Finally register the actual hooks

bp_lm = BPFileLastModified(fileLastModifiedSymbol)
bp_lm.silent = True

bp_write = BPWriteFile(writeFileSymbol)
bp_write.silent = True

bp_read = BPReadFile(readFileSymbol)
bp_read.silent = True


# Disconnection exit hook.
# We want GDB to restart and wait for a new connection with possibly new symbols after the
# inferior (ares) has been disconnected. We'll use an event for that, see
# https://sourceware.org/gdb/current/onlinedocs/gdb.html/Events-In-Python.html#Events-In-Python
#
# The libdragon toolchain's GDB installation doesn't ship with the Python libraries.
# That's why we need to import the events module directly (done normally in `python/lib/gdb/__init__.py`)

# import _gdbevents as events

# def exit_handler (event):
#   print("Disconnected. Quitting GDB.")
#   gdb.execute("quit")

# events.connection_removed.connect(exit_handler)
