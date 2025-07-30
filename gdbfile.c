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

#include "gdbfile.h"

#include <debug.h>
#include <n64sys.h>

#include <stdint.h>
#include <string.h>

#pragma GCC push_options
#pragma GCC optimize("O0")

struct DebuggerResponses {
    int read_bytes;
    int wrote_bytes;
    uint64_t last_modified;
};

static volatile struct DebuggerResponses __attribute__((aligned(16))) responses = {
    .read_bytes = -1,
    .wrote_bytes = -1,
    .last_modified = 0
};

void gdbfile_reset(void)
{
    responses.read_bytes = -1;
    responses.wrote_bytes = -1;
    responses.last_modified = 0;

    // These return values will be set by GDB in later function calls by writing to memory
    // so make sure they are evicted from the cache.
    static_assert(sizeof(responses) % 16 == 0);
    assert(((uint32_t)&responses) % 16 == 0);
    data_cache_hit_writeback_invalidate((void*)&responses, sizeof responses);
}

int gdbfile_read(const char* path, int max_size, uint8_t* dest)
{
    return responses.read_bytes;
}

int gdbfile_write(const char* path, const uint8_t* data, int data_size_bytes)
{
    return responses.wrote_bytes;
}

int gdbfile_write_string(const char* path, const char* data)
{
    return gdbfile_write(path, (const uint8_t*)data, strlen(data));
}

uint64_t gdbfile_last_modified(const char* path)
{
    return responses.last_modified;
}

#pragma GCC pop_options
