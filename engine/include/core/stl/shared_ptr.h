#pragma once

#include "core/memory.h"

namespace Sapfire::stl {

	template <typename T>
	using shared_ptr = std::shared_ptr<T>;

	//template <typename T, typename... Args>
	//SFAPI constexpr shared_ptr<T> make_shared(mem::ENUM category, Args&&... args) {
	//	return shared_ptr<T>(mem_new(category) T(std::forward<Args>(args)...), Deleter<T>{});
	//}
	template <typename T, typename... Args>
	SFAPI constexpr shared_ptr<T> make_shared(mem::ENUM category, Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
} // namespace Sapfire::stl
