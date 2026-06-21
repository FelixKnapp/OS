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
    printf("Formatted %%, %%c: %c, %%s: %s, %%ls: %ls\r\n", 'a', "string", far_str);
    printf("Formatted %%d: %d, %%i: %i,%%X: 0x%X, %%p: 0x%p, %%o: %o, %%hhd: %hd, %%hi: %hi, %%hhu: %hhu, %%hhd: %hhd\r\n", 1234, -5678, 0xdead, 0xbeef, 012345, (short)27, (short)-42, (unsigned char)20, (signed char)-10);
    printf("Formatted %%ld: %ld, %%lx: %lx, %%lld: %lld, %%llx: %llx,\r\n", -100000000l, 0xdeadbeeful, 10200300400ll, 0xdeadbeeffeebdaedull);
    // not in normal printf: special addictions (%p with uppercase letters and %b for binary output)
    printf("%%b: 0b%b, %%P: 0x%P, %%llX: 0x%X", 12, 0b1100, 0xdeadbeeffeebdaedull);
    for (;;);
}
