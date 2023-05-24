#include <core/logger.h>
#include <math/sfmath.h>

#define expect(left, right)                                                    \
	if (left != right) {                                                       \
		SF_FATAL ("--> Failed. Got %lld, but expected: %lld. File: %s:%d.",    \
				  left, right, __FILE__, __LINE__);                            \
		return FALSE;                                                          \
	}

#define expect_not(left, right)                                                \
	if (left == right) {                                                       \
		SF_FATAL ("--> Failed. Got %lld, but expected: %lld. File: %s:%d.",    \
				  left, right, __FILE__, __LINE__);                            \
		return FALSE;                                                          \
	}

#define expect_true(val)                                                       \
	if (!val) {                                                                \
		SF_FATAL ("--> Failed. Got true, but expected false. File: %s:%d.",    \
				  __FILE__, __LINE__);                                         \
		return FALSE;                                                          \
	}

#define expect_float(expected, actual)                                         \
	if (kabs (expected - actual) > 0.001f) {                                   \
		SF_FATAL ("--> Failed. Got %lld, but expected: %lld. File: %s:%d.",    \
				  expected, actual, __FILE__, __LINE__);                       \
		return FALSE;                                                          \
	}\
