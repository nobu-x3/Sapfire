#pragma once

#include "engpch.h"

namespace Sapfire
{

	template <typename T> using Scope = std::unique_ptr<T>;
	template <typename T, typename... Args> constexpr Scope<T> create_scope(Args &&...args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template <typename T> using Ref = std::shared_ptr<T>;
	template <typename T, typename... Args> constexpr Ref<T> create_ref(Args &&...args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	typedef uint32_t RendererID;

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
}