#include "stdint.h"
#include "stdio.h"
#include "stdbool.h"

void _cdecl cstart_(void)
{
    const char far* far_str = "far string";

    puts("Hello from the other side\n");
    puts("Quote Adele\r\n");
    puts("Hello world from C!\r\n");
    // printf testcases
    printf("Formatted %%, %c, %s, %ls\r\n", 'a', "string", far_str);
    printf("Formatted %d, %i, 0x%X, 0x%p, %o, %hd, %hi, %hhu, %hhd\r\n", 1234, -5678, 0xdead, 0xbeef, 012345, (short)27, (short)-42, (unsigned char)20, (signed char)-10);
    printf("Formatted %ld, %lx, %lld, %llx,\r\n", -100000000l, 0xdeadbeeful, 10200300400ll, 0xdeadbeeffeebdaedull);
    // not in normal printf: special addictions (%p with uppercase letters and %b for binary output)
    printf("0b%b, 0x%P", 12, 0b1100);
    for (;;);
}
