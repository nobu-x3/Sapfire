#include "test.h"
#include <stdio.h>
#include "core/logger.h"

void print_int(i32 num){
  printf("Num is %i\n", num);
  SF_DEBUG("TEST MESSAGE: %i", num);
}
