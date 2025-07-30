// Emulator host communication.
// The calls here are intercepted by file_hooks.py and values
// are filled in by the debugger.
//
// I've tried to disable optimizations for these functions because they
// look like NOPs to the compiler otherwise. 
//
// The return values are transferred in translation unit static variables.
// Local variables are difficult to set in a GDB breakpoint because the
// function is stopped at the beginning of the function, so any assignments
// would happen after breakpoint. And we need to assign return values in the
// case a function is called without debugger attached.
//
// That's why we offer a workaround 'reset()' function to clear them so that
// stale return values are never returned, even if the debugger disconnects
// when the ROM is running.

#include "gdb.hpp"

#include <debug.h>
#include <n64sys.h>

#include <cstdint>
#include <cstring>

#pragma GCC push_options
#pragma GCC optimize("O0")

alignas(16) static volatile struct ReturnValues {
    int gdbReadBytes = -1;
    int gdbWroteBytes = -1;
    uint64_t gdbLastModified = 0;
} returnValues;


static void writebackInvalidateReturnValues()
{
    static_assert(sizeof(returnValues) % 16 == 0);
    assert(((uint32_t)&returnValues) % 16 == 0);
    data_cache_hit_writeback_invalidate((void*)&returnValues, sizeof returnValues);
}

void GDB::reset()
{
    returnValues.gdbReadBytes = -1;
    returnValues.gdbWroteBytes = -1;
    returnValues.gdbLastModified = 0;

    // These return values will be set by GDB in later function calls by writing to memory
    // so make sure they are evicted from the cache.
    writebackInvalidateReturnValues();
}

int GDB::readFile(const char* path, int maxSize, uint8_t* dest)
{
    // debugf("readFile('%s', maxSize=%d, dest=%p)\n", path, maxSize, dest);
    return returnValues.gdbReadBytes;
}

int GDB::writeFile(const char* path, const uint8_t* data, int dataSizeBytes)
{
    // debugf("writeFileBytes('%s', data=%p, dataSizeBytes=%d)\n", path, data, dataSizeBytes);
    return returnValues.gdbWroteBytes;
}

int GDB::writeFileString(const char* path, const char* data)
{
    return writeFile(path, (uint8_t*)data, strlen(data));
}

uint64_t GDB::fileLastModified(const char* path)
{
    return returnValues.gdbLastModified;
}
#pragma GCC pop_options