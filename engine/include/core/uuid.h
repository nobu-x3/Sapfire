#pragma once

#include "core/core.h"

namespace Sapfire {

	class SFAPI UUID {
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;
		operator uint64_t() const { return m_UUID; }

	private:
		uint64_t m_UUID;
	};

} // namespace Sapfire

namespace std {
	template <typename T>
	struct hash;

	template <>
	struct hash<Sapfire::UUID> {
		std::size_t operator()(const Sapfire::UUID& uuid) const { return (uint64_t)uuid; }
	};

} // namespace std
