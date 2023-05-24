#pragma once

#include "defines.h"
#define SF_ASSERTIONS_ENABLED // TODO: move this to cmake
#ifdef SF_ASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak ()
#else
#define debugBreak() __builtin_trap ()
#endif

SAPI void report_assertion_failure (const char* expression, const char* message,
									const char* file, i32 line);

#define SF_ASSERT(expr, message)                                               \
	{                                                                          \
		if (expr) {                                                            \
		} else {                                                               \
			report_assertion_failure (#expr, message, __FILE__, __LINE__);     \
			debugBreak ();                                                     \
		}                                                                      \
	}

#ifdef _DEBUG
#define SF_DEBUG_ASSERT(expr)                                                  \
	{                                                                          \
		if (expr) {                                                            \
		} else {                                                               \
			report_assertion_failure (#expr, "", __FILE__, __LINE__);          \
			debugBreak ();                                                     \
		}                                                                      \
	}
#else
#define SF_DEBUG_ASSERT(expr)
#endif
#else
#define SF_ASSERT(expr, message)
#define SF_DEBUG_ASSERT(expr)
#endif
