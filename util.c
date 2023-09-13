/* 
    util.c 

    Function implementations for binary_to_hex and hex_to_binary. 
*/

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "util.h"
static char HEX[] = "0123456789abcdef";

/* 

    Binary to hex. 
    
    The resulting hex string will be in the form of a hexadecimal pair followed by
    a space, and a line for every 16 pair of digits. The memory will be 
    allocated similarly, but adding 2 extra spaces for the final new line and a 
    terminating null byte. The caller is responsible for calling free. 
    
*/

char *binary_to_hex(void *data, ssize_t n)
{
    int i, j;
    int hex_size = (3 * n) + (n / 16) + 2;
    char *hex_str, high_nibble, low_nibble; 
    
    /* Malloc space for hex string. */
    if ((hex_str = malloc(hex_size)) == NULL)
    {
        return NULL; 
    }

    /* Convert to hex and add a space. */
    for (i = 0, j = 0; i < n; i++) 
    {
        /* Cast to uint8_t (one byte = 8 bits, or 2 nibbbles). */
        uint8_t byte = ((uint8_t *)data)[i];

        /* Get the two nibbles and their corresponding hex. */ 
        high_nibble = HEX[byte >> 4];
        low_nibble  = HEX[byte & 0xf];

        /* Add corresponding hex digits and a space. */
        hex_str[j]   = high_nibble;
        hex_str[j+1] = low_nibble;
        hex_str[j+2] = ' ';
        j+=3; 

        /* Newline after every 16th pair of hex digits. */     
        if ((i + 1) % 16 == 0) 
        {   
            hex_str[j-1] = '\n'; 
        }
    }

    /* Add final new line and terminating null byte. */
    hex_str[j-1] = '\n'; 
    hex_str[j] = '\0'; 
    
    /* Return converted hex string. */
    return hex_str;
}

/* 

    Hex to binary.
    
    Ignores if the buffer contains an odd number of hex digits. 
    Also ignores whitspace. Assumes hex is null terminated. 

*/

void *hex_to_binary(char *hex)
{
    int i, j; 
    int max_bin = ((strlen(hex)) / 2) + 1;
    uint8_t *bin_buf;
    
    /* Malloc for the bin_buf. */
    if ((bin_buf = malloc(max_bin)) == NULL)
    {
        return NULL; 
    }

    /* Convert to binary if hex value. */
    for (i = 0, j = 0; i < strlen(hex); i++)
    {
        /* Skip whitespace. */
        if (isspace(hex[i]))
        {
            continue; 
        }

        /* Check if 0-9 or A-F/a-f. If it is, convert
           from ASCII to integer value using strtol(). */
        char c = hex[i];
        if (isdigit(c) || isalpha(c)) 
        {
            c = strtol(&c, NULL, 16);
        }
        else
        {
            free(bin_buf);
            return NULL;
        }

        /* If low nibble in pair (j is odd), bitwise OR (|). 
           If the high nibble (j is even), bitwise left shift << it.  */
        if (j % 2 == 0)
        {
            bin_buf[j/2] = (c << 4);
        }
        else
        {
            bin_buf[j/2] |= c;
        }
        j++;
    }

    bin_buf[(j/2)] = '\0';
    return (void *) bin_buf;
}
