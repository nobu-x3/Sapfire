#pragma once

#include "components/component.h"
#include "core/rtti.h"

namespace Sapfire::components {

	class NameComponent {
		RTTI;
		ENGINE_COMPONENT(NameComponent);

	public:
		void register_rtti();
		NameComponent();
        NameComponent(const NameComponent&);
        NameComponent(NameComponent&&) noexcept;
        NameComponent& operator=(const NameComponent&);
        NameComponent& operator=(NameComponent&&) noexcept;
		NameComponent(stl::string_view name);
		inline const stl::string& name() const { return m_Name; }
		inline void name(const stl::string& name) { m_Name = name; }

	private:
		stl::string m_Name;
	};
} // namespace Sapfire
