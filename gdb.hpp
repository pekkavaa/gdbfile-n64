#pragma once

#include <cstdint>

// We want to hook to these functions via GDB so it's cleaner to not inline them.
#define SNUG_NOINLINE __attribute__ ((noinline))

namespace GDB {
// Clears cached return values. A workaround for stale return values that may happen
// if GDB connection is lost during a session.
 void SNUG_NOINLINE reset();

// Reads file from host to buffer 'dest'.
// Returns -1 on failure, and the number of read bytes on success.
int SNUG_NOINLINE readFile(const char* path, int maxSize, uint8_t* dest);

// Writes 'data' to the given file on the host.
// Returns -1 on failure, the number of bytes written otherwise.
int SNUG_NOINLINE writeFile(const char* path, const uint8_t* data, int dataSizeBytes);

// Writes a string to the given file.
// Returns -1 on failure, the number of bytes written otherwise.
int SNUG_NOINLINE writeFileString(const char* path, const char* data);

// Returns the last modification time of 'path' in milliseconds since the Unix epoch.
// Returns 0 on failure.
uint64_t SNUG_NOINLINE fileLastModified(const char* path);
}