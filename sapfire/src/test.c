#include "test.h"
#include <stdio.h>
#include "core/logger.h"
#include "core/asserts.h"

void print_int(i32 num){
  printf("Num is %i\n", num);
  SF_ASSERT(1 == 0, "Sure failure");
  SF_DEBUG("TEST MESSAGE: %i", num);
}
