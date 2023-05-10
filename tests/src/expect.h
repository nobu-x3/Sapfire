#include <core/logger.h>
#include <math/sfmath.h>

#define expect(left, right) 																																						\
	if(left != right){																																										\
		SF_ERROR("--> Failed. Expected %lld, but got: %lld. File: %s:%d.", 																	\
			left, right, __FILE__, __LINE__);																																	\
			return FALSE;																																											\
	}																																																			\
	else{																																																	\
		SF_INFO("Passed: Test in %s:%d.", __FILE__, __LINE__);																							\
	}																																																			\

#define expect_true(val)																																								\
if(!val){																																																\
			SF_ERROR("--> Failed. Expected true, but got false. File: %s:%d.", 																\
			 __FILE__, __LINE__);																																							\
			return FALSE;																																											\
}																																																				\
else {																																																	\
		SF_INFO("Passed test in %s:%d.", __FILE__, __LINE__);																								\
}


#define expect_float(expected, actual)                                                        					\
    if (kabs(expected - actual) > 0.001f) {                                                         		\
			SF_ERROR("--> Failed. Expected %lld, but got: %lld. File: %s:%d.", 																\
			expected, actual, __FILE__, __LINE__);																														\
			return FALSE;																																											\
    }																																																		\
		else {																																															\
				SF_INFO("Passed test in %s:%d.", __FILE__, __LINE__);																						\
		}