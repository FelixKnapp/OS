#include "string.h"
#include "stdbool.h"
#include "stdint.h"

// Find first occurence of a char in a string and returns pointer to that char
const char* strchr(const char* str, char chr)
{
    if (str == NULL) return NULL;

    while(*str)
    {
        if(*str == chr) return str;

        ++str;
    }

    return NULL;
}

// Returns length of a string
unsigned strlen(char* str)
{
    if(str == NULL) return 0;

    unsigned len = 0;

    while (*str)
    {
        len++;
        str++;
    }

    return len;
}

// Copies second string into first string
char* strcpy(char* str_dst, const char* str_src)
{
    if(str_dst == NULL) return NULL;
    if(str_src == NULL)
    {
        *str_dst = '\0';
        return str_dst;
    }

    const char* str_dst_og = str_dst;

    while(*str_src)
    {
        *str_dst = *str_src; 
        ++str_dst; 
        ++str_src;
    }

    return str_dst_og;
}
