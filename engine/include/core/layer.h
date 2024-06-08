#pragma once

#include "events/event.h"

namespace Sapfire {
	class Layer {
	public:
		Layer(stl::string_view name) : m_DebugName(name){};
		virtual ~Layer() = default;
		virtual void on_attach(){};
		virtual void on_detach(){};
		virtual void on_update(float dt){};
		virtual void on_event(Event& event) {}
        virtual void on_render(){};
		inline stl::string_view get_name() const { return m_DebugName; }

	protected:
		stl::string m_DebugName;
	};
} // namespace Sapfire
