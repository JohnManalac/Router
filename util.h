#ifndef util__H
#define util__H

#include <stddef.h>
#include <sys/types.h>

char *binary_to_hex(void *data, ssize_t n);
void *hex_to_binary(char *hex);

#endif /* util__H */
