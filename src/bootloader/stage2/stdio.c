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
                if(*fmp == 'h'){
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                } else goto PRINTF_STATE_SPEC_;
                break;

            case PRINTF_STATE_LENGTH_LONG:
                if(*fmp == 'l'){
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
            length = PRINTF_STATE_DEFAULT;
            radix = 10;
            sign = false;
            break;
            
        }

        fmt++;
    }
    
}

int* printf_number(int* printf, int length, bool sign, radix) {

}