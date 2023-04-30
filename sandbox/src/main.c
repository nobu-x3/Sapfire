#include <platform/platform.h>
#include <test.h>

#include "core/logger.h"

int main(void) {
  platform_state state;
  if (platform_init(&state, "Sapfire Engine Test", 100, 100, 800, 600, 0)) {
    platform_update_internal_state(&state);
  }
  platform_shutdown(&state);
  return 0;
}
