#ifndef frame_crc32__H
#define frame_crc32__H

#include <sys/types.h>
#include <stdint.h>

/* Function Prototypes. */
uint32_t crc32(uint32_t crc, const void *buf, size_t size);

#endif /* frame_crc32__H */
