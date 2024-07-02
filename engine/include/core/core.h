#pragma once

#include <array>
#include <bitset>
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <filesystem>
#include "core/platform.h"
#ifdef SF_PLATFORM_WINDOWS
#ifdef SF_BUILD_DLL
#define SFAPI __declspec(dllexport)
#else
#define SFAPI __declspec(dllimport)
#endif
#else
#error Sapfire only supports Windows!
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif

namespace Sapfire {

	using u8 = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;
	using s8 = int8_t;
	using s16 = int16_t;
	using s32 = int32_t;
	using s64 = int64_t;
	using f32 = float;
	using f64 = double;
	using RendererID = u32;

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

#define BIND_EVENT_FN_FOR_OBJ(o, x) std::bind(&x, o, std::placeholders::_1)

	namespace stl {
		template <class _This, class... _Rest>
		using tuple = SFAPI std::tuple<_This, _Rest...>;
		template <class _Ty>
		using reference_wrapper = SFAPI std::reference_wrapper<_Ty>;
		template <typename _Ty, class _Container = std::deque<_Ty>>
		using queue = SFAPI std::queue<_Ty, _Container>;

		template <typename T>
		using function = SFAPI std::function<T>;
		using string = SFAPI std::string;
		using string_view = SFAPI std::string_view;
		using stringstream = SFAPI std::stringstream;
		template <typename T>
		using vector = SFAPI std::vector<T>;
		using mutex = SFAPI std::mutex;
		template <typename Mutex>
		using lock_guard = SFAPI std::lock_guard<Mutex>;
		template <typename Mutex>
		using unique_lock = SFAPI std::unique_lock<Mutex>;
		template <typename Mutex>
		using scoped_lock = SFAPI std::scoped_lock<Mutex>;
		using thread = SFAPI std::thread;
		using jthread = SFAPI std::jthread;
		template <typename T>
		using optional = SFAPI std::optional<T>;
		template <typename T, std::size_t Extent = std::dynamic_extent>
		using span = SFAPI std::span<T, Extent>;
		template <class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>,
				  class _Alloc = std::allocator<std::pair<const _Kty, _Ty>>>
		using unordered_map = SFAPI std::unordered_map<_Kty, _Ty, _Hasher, _Keyeq, _Alloc>;
		template <class Key, class T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key, T>>>
		using map = SFAPI std::map<Key, T, Compare, Allocator>;
		template <class _Ty, size_t _Size>
		using array = SFAPI std::array<_Ty, _Size>;
		using wstring = SFAPI std::wstring;
		using wstring_view = SFAPI std::wstring_view;
		using recursive_mutex = SFAPI std::recursive_mutex;
		template <size_t Size>
		using bitset = SFAPI std::bitset<Size>;

		struct SFAPI generational_index {
			u32 index = 0;
			u32 generation = 0;
		};

		class SFAPI generational_index_allocator {
		public:
			generational_index allocate() {
				if (m_FreeIndices.size() > 0) {
					u32 index = m_FreeIndices.back();
					m_FreeIndices.pop_back();
					m_Entries[index].generation += 1;
					m_Entries[index].is_alive = true;
					return {index, m_Entries[index].generation};
				}
				m_Entries.push_back({true, 0});
				return {static_cast<u32>(m_Entries.size()) - 1, 0};
			}

			void deallocate(generational_index index) {
				if (is_alive(index)) {
					m_Entries[index.index].is_alive = false;
					m_FreeIndices.push_back(index.index);
				}
			}

			bool is_alive(generational_index index) const {
				return index.index < m_Entries.size() && m_Entries[index.index].generation == index.generation &&
					m_Entries[index.index].is_alive;
			}

		private:
			struct entry {
				bool is_alive = false;
				u32 generation = 0;
			};
			stl::vector<entry> m_Entries{};
			stl::vector<u32> m_FreeIndices{};
		};

		template <typename T>
		class SFAPI generational_vector {
		public:
			struct entry {
				u32 generation;
				T value;
			};

			void set(generational_index index, T val) {
				while (m_Entries.size() <= index.index)
					m_Entries.push_back(std::nullopt);
				u32 prev_gen = 0;
				if (auto prev_entry = m_Entries[index.index])
					prev_gen = prev_entry->generation;
				if (prev_gen > index.generation) {
					std::cerr << "Cannot set value at index" << index.index
							  << ": previous generation is larger than current generation:" << prev_gen << ">" << index.generation << "."
							  << std::endl;
					return;
				}
				m_Entries[index.index] = optional<entry>{{
					.generation = index.generation,
					.value = val,
				}};
			}

			void remove(generational_index index) {
				if (index.index < m_Entries.size()) {
					m_Entries[index.index] = std::nullopt;
				}
			}

			[[nodiscard]] T* get(generational_index index) {
				if (index.index >= m_Entries.size())
					return nullptr;
				if (auto& entry = m_Entries[index.index]) {
					if (entry->generation == index.generation)
						return &entry->value;
				}
				return nullptr;
			}

			vector<generational_index> get_all_valid_indices(const generational_index_allocator& allocator) const {
				vector<generational_index> result;
				for (u32 i = 0; i < m_Entries.size(); ++i) {
					const auto& entry = m_Entries[i];
					if (!entry)
						continue;
					generational_index index = {i, entry->generation};
					if (allocator.is_alive(index)) {
						result.push_back(index);
					}
				}
				return result;
			}

			size_t size() const { return m_Entries.size(); }

			stl::optional<stl::tuple<generational_index, stl::reference_wrapper<const T>>>
			get_first_valid_entry(const generational_index_allocator& allocator) {
				for (auto i = 0; i < m_Entries.size(); ++i) {
					const auto& entry = m_Entries[i];
					if (!entry)
						continue;
					generational_index index = {i, entry->generation};
					if (allocator.is_alive(index)) {
						return std::make_tuple(index, std::ref(entry->value));
					}
				}
				return std::nullopt;
			}

			auto begin() { return m_Entries.begin(); }
			auto end() { return m_Entries.end(); }
			auto cbegin() { return m_Entries.cbegin(); }
			auto cend() { return m_Entries.cend(); }
			auto begin() const { return m_Entries.begin(); }
			auto end() const { return m_Entries.end(); }

		private:
			stl::vector<stl::optional<entry>> m_Entries;
		};

	} // namespace stl

} // namespace Sapfire
