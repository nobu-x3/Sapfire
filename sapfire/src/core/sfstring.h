#pragma once

#include "defines.h"

SAPI u64 sfstrlen(const char* string);
SAPI char* sfstrdup(const char* string);
SAPI b8 sfstreq(const char* a, const char* str1);
SAPI i32 sfstrfmt(char* dest, const char* format, ...);