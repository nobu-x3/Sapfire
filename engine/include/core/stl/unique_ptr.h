#pragma once

#include <memory>
#include "core/memory.h"

namespace Sapfire::stl {

	template <typename T, typename D = Deleter<T>>
	using unique_ptr = std::unique_ptr<T, D>;

	template <typename T, typename... Args>
	SFAPI constexpr unique_ptr<T> make_unique(Sapfire::mem::ENUM category, Args&&... args) {
		return unique_ptr<T>(mem_new(category) T(std::forward<Args>(args)...));
	}
} // namespace Sapfire::stl
