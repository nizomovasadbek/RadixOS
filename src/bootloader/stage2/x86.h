#ifndef _X86_H_WCC_LIBRARY
#define _X86_H_WCC_LIBRARY

#include "stdint.h"

void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotientOut, uint32_t* reminderOut);
void _cdecl x86_VideoWriteChatTeletype(char c, uint8_t page);

#endif