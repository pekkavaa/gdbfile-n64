/**
 * @file gdbfile.h
 * @brief Host filesystem access via functions that a GDB script hooks to.
 */
#ifndef GDBFILE_H
#define GDBFILE_H

/* The hooks won't work if functions are inlined. */
#define GDBFILE_NOINLINE __attribute__ ((noinline))

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Clears cached return values.
 *
 * A workaround for stale return values that may happen if GDB connection is lost during a session.
 */
void GDBFILE_NOINLINE gdbfile_reset(void);

/**
 * @brief Reads file from host to buffer 'dest'.
 *
 * @param[in]  path
 *             Path to the file to read.
 * @param[in]  max_size_bytes
 *             Maximum number of bytes to read.
 * @param[out] dest
 *             Buffer to store the read bytes.
 *
 * @return -1 on failure, and the number of read bytes on success.
 */
int GDBFILE_NOINLINE gdbfile_read(const char* path, int max_size_bytes, uint8_t* dest);

/**
 * @brief Writes 'data' to the given file on the host.
 *
 * @param[in]  path
 *             Path to the file to write.
 * @param[in]  data
 *             Data to write to the file.
 * @param[in]  data_size_bytes
 *             Number of bytes to write.
 *
 * @return -1 on failure, the number of bytes written otherwise.
 */
int GDBFILE_NOINLINE gdbfile_write(const char* path, const uint8_t* data, int data_size_bytes);

/**
 * @brief Writes a string to the given file.
 *
 * @param[in]  path
 *             Path to the file to write.
 * @param[in]  data
 *             String to write to the file.
 *
 * @return -1 on failure, the number of bytes written otherwise.
 */
int GDBFILE_NOINLINE gdbfile_write_string(const char* path, const char* data);

/**
 * @brief Returns the last modification time of 'path' in milliseconds since the Unix epoch.
 *
 * @param[in]  path
 *             Path to the file.
 *
 * @return 0 on failure, the last modification time of 'path' in milliseconds since the Unix epoch otherwise.
 */
uint64_t GDBFILE_NOINLINE gdbfile_last_modified(const char* path);

#undef GDBFILE_NOINLINE

#ifdef __cplusplus
}
#endif

#endif // GDBFILE_H
