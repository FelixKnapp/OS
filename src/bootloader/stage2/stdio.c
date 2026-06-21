#include "stdio.h"
#include "x86.h"

void putc(char c)
{
    x86_Video_WriteCharTeletype(c, 0);
}

void puts(const char *str)
{
    while(*str)
    {
        putc(*str);
        str++;
    }
}

// print far string
void puts_f(const char far* str)
{
    while(*str)
    {
        putc(*str);
        str++;
    }
}

#define PRINTF_STATE_NORMAL         0
#define PRINTF_STATE_LENGTH         1
#define PRINTF_STATE_LENGTH_SHORT   2
#define PRINTF_STATE_LENGTH_LONG    3
#define PRINTF_STATE_SPEC           4

#define PRINTF_LENGTH_DEFAULT       0
#define PRINTF_LENGTH_SHORT_SHORT   1
#define PRINTF_LENGTH_SHORT         2
#define PRINTF_LENGTH_LONG          3
#define PRINTF_LENGTH_LONG_LONG     4

int* printf_number(int* argp, int length, bool sign, uint8_t radix, bool small_hex);

void _cdecl printf(const char *fmt, ...)
{
    int *argp = (int*)&fmt;
    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    uint8_t radix = 10;
    bool sign = false;

    argp++;

    while(*fmt)
    {
        switch (state)
        {
            // look for argument or print
            case PRINTF_STATE_NORMAL:
                switch (*fmt)
                {
                    case '%':   state = PRINTF_STATE_LENGTH;
                                break;

                    default:    putc(*fmt);
                                break;
                } 
                break;
            
            // length argument
            case PRINTF_STATE_LENGTH:
                switch (*fmt)
                {
                    case 'h':   length = PRINTF_LENGTH_SHORT;
                                state = PRINTF_STATE_LENGTH_SHORT;
                                break;
                    case 'l':   length = PRINTF_LENGTH_LONG;
                                state = PRINTF_STATE_LENGTH_LONG;
                                break;
                    default:    goto PRINTF_STATE_SPEC_;
                }
                break;

            // length argument = short
            case PRINTF_STATE_LENGTH_SHORT:
                if(*fmt == 'h')
                {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                }
                else
                {
                    goto PRINTF_STATE_SPEC_;
                }
                break;

            // length argument = long
            case PRINTF_STATE_LENGTH_LONG:
                if(*fmt == 'h')
                {
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPEC;
                }
                else
                {
                    goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_SPEC:
            PRINTF_STATE_SPEC_:
                switch (*fmt)
                {
                                //print char
                    case 'c':   putc((char)*argp);
                                argp++;
                                break;

                                // print string
                    case 's':   if (length == PRINTF_LENGTH_LONG || length == PRINTF_LENGTH_LONG_LONG) 
                                {
                                    // far pointer
                                    puts_f(*(const char far**)argp);
                                    argp += 2;
                                }
                                else 
                                {
                                    // near pointer
                                    puts(*(const char**)argp);
                                    argp++;
                                }
                                break;

                                // print % symbol
                    case '%':   putc('%');
                                break;

                                // print signed integer value
                    case 'd':
                    case 'i':   radix = 10; sign = true;
                                argp = printf_number(argp, length, sign, radix, true);
                                break;

                                // print unsigned integer value 
                    case 'u':   radix = 10; sign = false;
                                argp = printf_number(argp, length, sign, radix, true);
                                break;

                                //
                                //  NOT IN NORMAL PRINTF
                                //

                                // print memory adress large
                    case 'P':
                                // print integer in hexadecimal large 
                    case 'X':   radix = 16; sign = false;
                                argp = printf_number(argp, length, sign, radix, false);
                                break;
                                
                                // print integer in hexadecimal small
                    case 'x':
                                // print memory adress small
                    case 'p':   radix = 16; sign = false;
                                argp = printf_number(argp, length, sign, radix, true);
                                break;

                                // print integer in octal 
                    case 'o':   radix = 8; sign = false;
                                argp = printf_number(argp, length, sign, radix,  true);
                                break;
                                
                                //
                                //  NOT IN NORMAL PRINTF
                                //

                                // print integer in binary
                    case 'b':   radix = 2; sign = false;
                                argp = printf_number(argp, length, sign, radix,  true);
                                break;

                    // ignore invalid spec
                    default:    break;
                }
            
                // reset states 
                state = PRINTF_STATE_NORMAL;
                length = PRINTF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                break;
        }
        
        fmt++;
    }
}

const char g_hexchars_small[] = "0123456789abcdef";
const char g_hexchars_large[] = "0123456789ABCDEF";

int* printf_number(int* argp, int length, bool sign, uint8_t radix, bool small_hex)
{
    char buffer[32];
    unsigned long long number;
    int number_sign = 1;
    int pos = 0;

    // process length
    switch (length)
    {
        case PRINTF_LENGTH_SHORT_SHORT:
        case PRINTF_LENGTH_SHORT:
        case PRINTF_LENGTH_DEFAULT:
            if (sign)
            {
                int n = *argp;
                if (n < 0)
                {
                    n = -n;
                    number_sign = -1;
                }
                number = (unsigned long long)n;
            }
            else
            {
                number = *(unsigned int*)argp;
            }
            argp++;
            break;

        case PRINTF_LENGTH_LONG:
            if (sign)
            {
                long int n = *(long int*)argp;
                if (n < 0)
                {
                    n = -n;
                    number_sign = -1;
                }
                number = (unsigned long long)n;
            }
            else
            {
                number = *(unsigned long int*)argp;
            }
            argp += 2;
            break;

        case PRINTF_LENGTH_LONG_LONG:
            if (sign)
            {
                long long int n = *(long long int*)argp;
                if (n < 0)
                {
                    n = -n;
                    number_sign = -1;
                }
                number = (unsigned long long)n;
            }
            else
            {
                number = *(unsigned long long int*)argp;
            }
            argp += 4;
            break;
    }

    // convert number to ASCII
    do 
    {
        uint32_t rem;
        x86_div64_32(number, radix, &number, &rem);
        if(small_hex)
        {
            buffer[pos++] = g_hexchars_small[rem];
        }
        else
        {
            buffer[pos++] = g_hexchars_large[rem];
        }
    } while (number > 0);

    // add sign
    if (sign && number_sign < 0)
        buffer[pos++] = '-';

    // print number in reverse order
    while (--pos >= 0)
        putc(buffer[pos]);

    return argp;
}
