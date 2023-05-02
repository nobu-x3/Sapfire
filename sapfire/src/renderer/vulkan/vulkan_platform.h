#pragma once

#include "defines.h"

struct platform_state;

// void platform_get_required_extension_names(struct platform_state *plat_state,
// 										   u32 *count, const char **names);

void platform_get_required_extension_names(struct platform_state *plat_state,
										   const char*** names_vec);
