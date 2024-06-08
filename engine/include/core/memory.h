#pragma once

#include "core/core.h"

namespace Sapfire::mem {
	enum ENUM : u16 {
		Engine_Rendering = 0x0,
		Engine_Physics = 0b0000000000000010,
		Engine_Components = 0b0000000000000100,
		Engine_Scene = 0b0000000000001000,
		Engine_Strings = 0b0000000000010000,
		Engine_MeshManager = 0b0000000000100000,
		Engine_TextureManager = 0b0000000001000000,
		Engine_MaterialManager = 0b0000000010000000,
		Engine_AI = 0b0000000100000000,
		Game_Components = 0b0000001000000000,
		Game_AI = 0b0000010000000000,
		Editor = 0b0000100000000000,
		Engine_Core = 0b0001000000000000,
		Engine_Input = 0b0010000000000000,
		Engine_FileSystem = 0b0100000000000000
	};

	std::ostream& operator<<(std::ostream& os, const ENUM& category);
	void alloc(ENUM, size_t);
	void dealloc(ENUM, size_t);
	void memdump();
	Sapfire::stl::unordered_map<Sapfire::mem::ENUM, size_t>& categories();
} // namespace Sapfire::mem

#if defined(DEBUG) | defined(_DEBUG)
/* [[nodiscard]] void* __cdecl operator new(size_t size, Sapfire::mem::ENUM category); */
/* void delete_memory(void* address); */
/* #define mem_new(category) new (Sapfire::mem::ENUM::Engine_Core) */
/* #define mem_delete(thing) \ */
/* 	delete_memory(reinterpret_cast<void*>(thing)); \ */
/* 	thing = nullptr */
#define mem_new(category) new
#define mem_delete(thing) delete thing
#else
#define mem_new(category) new
#define mem_delete(thing) delete thing
#endif
/* void operator delete(void*); */

namespace Sapfire::stl {
	template <typename T>
	struct Deleter {
		constexpr Deleter() noexcept = default;

		template <class T2, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
		constexpr Deleter(const Deleter<T2>&) noexcept {}

		void operator()(T* p) const {
			static_assert(0 < sizeof(T), "can't delete an incomplete type");
			mem_delete(p);
		}
	};
} // namespace Sapfire::stl
