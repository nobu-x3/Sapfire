#pragma once

#include "core/core.h"

namespace Sapfire {

	class Layer;

	class LayerStack {
	public:
		LayerStack() = default;
		~LayerStack();
		void push_layer(Layer* layer);
		void pop_layer(Layer* layer);
		void push_overlay(Layer* overlay);
		void pop_overlay(Layer* overlay);
		inline stl::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		inline stl::vector<Layer*>::iterator end() { return m_Layers.end(); }
		inline stl::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		inline stl::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }
		inline stl::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
		inline stl::vector<Layer*>::const_iterator end() const { return m_Layers.end(); }
		inline stl::vector<Layer*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		inline stl::vector<Layer*>::const_reverse_iterator rend() const { return m_Layers.rend(); }

	private:
		stl::vector<Layer*> m_Layers;
		u32 m_LayerInsertIndex = 0;
	};
} // namespace Sapfire
