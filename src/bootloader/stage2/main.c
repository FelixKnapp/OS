#include "stdint.h"
#include "stdio.h"

void _cdecl cstart_(void)
{
    puts("Hello from the other side\r\n");
    puts("Quote Adele\r\n");
    for (;;);
}
