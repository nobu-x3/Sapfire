#include "engpch.h"

#include "core/layer.h"
#include "core/layer_stack.h"

namespace Sapfire {

	LayerStack::~LayerStack() {
		for (auto* layer : m_Layers) {
			layer->on_detach();
			delete layer;
		}
	}

	void LayerStack::push_layer(Layer* layer) {
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::pop_layer(Layer* layer) {
		auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
		if (it != m_Layers.begin() + m_LayerInsertIndex) {
			layer->on_detach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::push_overlay(Layer* overlay) { m_Layers.emplace_back(overlay); }

	void LayerStack::pop_overlay(Layer* overlay) {
		auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);
		if (it != m_Layers.end()) {
			overlay->on_detach();
			m_Layers.erase(it);
		}
	}
} // namespace Sapfire
