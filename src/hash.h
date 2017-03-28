

#ifndef __HASH__H__
#define __HASH__H__


#include "config.h"


uint64_t siphash(const uint8_t *in, const size_t inlen, const uint8_t *k);
uint64_t siphash_nocase(const uint8_t *in, const size_t inlen, const uint8_t *k);


#endif
