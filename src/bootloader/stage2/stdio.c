#include "stdio.h"
#include "x86.h"

void putc(char c){
    x86_VideoWriteChatTeletype(c, 0);
}

void puts(const char* str){
    while(*str){
        putc(*str);
        *str++;
    }
}

void _cdecl printf(const char *fmt, ...){
    int* argp = (int*) &fmt;
    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;

    argp++;

    while(*fmt) {

        switch(state) {
            case PRINTF_STATE_NORMAL:
                switch(*fmt) {
                    case '%':
                        state = PRINTF_STATE_LENGTH;
                        break;
                    default:
                        putc(*fmt);
                        break;
                }
                break;
            case PRINTF_STATE_LENGTH:
                switch(*fmt) {
                    case 'h':
                        length = PRINTF_LENGTH_SHORT;
                        state = PRINTF_STATE_LENGTH_SHORT;
                        break;
                    case 'l':
                        length = PRINTF_LENGTH_LONG;
                        state = PRINTF_STATE_LENGTH_LONG;
                        break;

                    default:
                        goto PRINTF_STATE_SPEC_;
                }
                break;
            case PRINTF_STATE_LENGTH_SHORT:
                if(*fmt == 'h'){
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                } else goto PRINTF_STATE_SPEC_;
                break;

            case PRINTF_STATE_LENGTH_LONG:
                if(*fmt == 'l'){
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPEC;
                } else goto PRINTF_STATE_SPEC_;
                break;
            case PRINTF_STATE_SPEC:
            PRINTF_STATE_SPEC_:
                switch(*fmt) {
                    case 'c': putc((char) *argp);
                    argp++;
                    break;

                    case 's': puts(*(char**) argp);
                    argp++;
                    break;

                    case '%': puts('%');
                    break;

                    case 'd':
                    case 'i': sign = true;
                    radix = 10;
                    argp = printf_number(argp, length, sign, radix);
                    break;

                    case 'u':
                        sign = false;
                        radix = 10;
                        argp = printf_number(argp, length, sign, radix);
                        break;

                    case 'x':
                    case 'X':
                    case 'p':
                        sign = false;
                        radix = 16;
                        argp = printf_number(argp, length, sign, radix);

                    case 'o':
                        sign = false;
                        radix = 8;
                        argp = printf_number(argp, length, sign, radix);
                        break;

                    default: break;
                }

            state = PRINTF_STATE_NORMAL;
            length = PRINTF_LENGTH_DEFAULT;
            radix = 10;
            sign = false;
            break;
            
        }

        fmt++;
    }
    
}

const char g_HexChars[] = "0123456789abcdef";

int* printf_number(int* argp, int length, bool sign, int radix) {
    char buffer[32];
    unsigned long long number;
    int number_sign = 1;
    int pos = 0;

    switch (length) {
        case PRINTF_LENGTH_SHORT_SHORT:
        case PRINTF_LENGTH_SHORT:
        case PRINTF_LENGTH_DEFAULT:
            if(sign) {
                int n = *argp;
                if(n < 0) {
                    n = -n;
                    number_sign = 1;
                }
                number = (unsigned long long) n;
            }
            else {
                number = *(unsigned long*) argp;
            }
            argp++;
            break;

        case PRINTF_LENGTH_LONG:
            if(sign) {
                long int n = *(long int*) argp;
                if (n < 0) {
                    n = -n;
                    number_sign = 1;
                }
                number = (unsigned long long) n;
            }
            else {
                number = *(unsigned long int*) argp;
            }
            argp += 2;
            break;

        case PRINTF_LENGTH_LONG_LONG:
            if(sign) {
                long long int n = *(long long int*) argp;
                if(n < 0) {
                    n = -n;
                    number_sign = 1;
                }
                number = (unsigned long long) n;
            } else {
                number = *(unsigned long long*) argp;
            }
            argp += 4;
            break;
    }
}