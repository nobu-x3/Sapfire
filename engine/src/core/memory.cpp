#include "engpch.h"

#include <cstdlib>
#include "core/memory.h"

Sapfire::stl::unordered_map<Sapfire::mem::ENUM, size_t> g_CategorySizes{};

namespace Sapfire::mem {

	Sapfire::stl::unordered_map<Sapfire::mem::ENUM, size_t>& categories() { return g_CategorySizes; }

	void memdump() {
#if defined(DEBUG) | defined(_DEBUG)
		for (auto&& [category, size] : g_CategorySizes) {
			std::cout << category << "\t\t: " << size << std::endl;
		}
#endif
	}

	void alloc(ENUM cat, size_t size) { g_CategorySizes[cat] += size; }
	void dealloc(ENUM cat, size_t size) { g_CategorySizes[cat] -= size; }
	std::ostream& operator<<(std::ostream& os, const ENUM& category) {
		switch (category) {
		case ENUM::Engine_Core:
			os << "Engine Core";
			break;
		case ENUM::Engine_Rendering:
			os << "Engine Rendering";
			break;
		case ENUM::Engine_Physics:
			os << "Engine Physics";
			break;
		case ENUM::Engine_Components:
			os << "Engine Components";
			break;
		case ENUM::Engine_Scene:
			os << "Engine Scene";
			break;
		case ENUM::Engine_AI:
			os << "Engine AI";
			break;
		case ENUM::Engine_Strings:
			os << "Engine Strings";
			break;
		case ENUM::Game_AI:
			os << "Game AI";
			break;
		case ENUM::Game_Components:
			os << "Game Components";
			break;
		case ENUM::Engine_MeshManager:
			os << "Engine Mesh Manager";
			break;
		case ENUM::Engine_TextureManager:
			os << "Engine Texture Manager";
			break;
		case ENUM::Engine_MaterialManager:
			os << "Engine Material Manager";
			break;
		case ENUM::Editor:
			os << "Editor";
			break;
		case ENUM::Engine_Input:
			os << "Engine Input";
			break;
		case ENUM::Engine_FileSystem:
			os << "Engine File System";
			break;
		}
		return os;
	}
} // namespace Sapfire::mem

constexpr size_t CATEGORY_BYTE_LENGTH = sizeof(Sapfire::mem::ENUM) / sizeof(char);
constexpr size_t SIZE_BYTE_LENGTH = sizeof(size_t) / sizeof(char);

void* operator new(size_t size, Sapfire::mem::ENUM category) {
	alloc(category, size);
	void* address = malloc(sizeof(Sapfire::mem::ENUM) + sizeof(size_t) + size);
	auto* cat = reinterpret_cast<Sapfire::mem::ENUM*>(address);
	*cat = category;
	auto* mem = reinterpret_cast<char*>(address);
	mem += CATEGORY_BYTE_LENGTH;
	auto* p_size = reinterpret_cast<size_t*>(mem);
	*p_size = size;
	mem += SIZE_BYTE_LENGTH;
	return reinterpret_cast<void*>(mem);
}

void delete_memory(void* address) {
	char* mem = reinterpret_cast<char*>(address);
	char* p_size_addr = mem - SIZE_BYTE_LENGTH;
	char* p_cat_addr = p_size_addr - CATEGORY_BYTE_LENGTH;
	size_t* p_size = reinterpret_cast<size_t*>(p_size_addr);
	const Sapfire::mem::ENUM cat = *reinterpret_cast<Sapfire::mem::ENUM*>(p_cat_addr);
	mem -= sizeof(Sapfire::mem::ENUM) / sizeof(char);
	dealloc(cat, *p_size);
	free(p_cat_addr);
    address = nullptr;
}
