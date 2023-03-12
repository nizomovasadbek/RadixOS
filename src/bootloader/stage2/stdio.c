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

        }

        fmt++;
    }
    
}