#include "utility.h"
#include "stdint.h"

uint32_t align(uint32_t number, uint32_t alignTo) {
    if(!alignTo) {
        return number;
    }

    uint32_t rem = number % alignTo;
    return (rem > 0) ? (number + alignTo - rem) : number;
}