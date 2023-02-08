#pragma once

#include "Sapfire/core/Core.h"
#include "Sapfire/core/Layer.h"

namespace Sapfire
{
	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		// TODO: maybe pop by index? altho i probably don't need to as there's rarely a need to do it in runtime.
		// Usually it's the whole stack...
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);

		// TODO: will need reverse iterators too
		std::vector<Layer*>::iterator begin() { return mLayers.begin(); }
		std::vector<Layer*>::iterator end() { return mLayers.end(); }

	private:
		std::vector<Layer*> mLayers;
		unsigned int mLayerInsertIndex = 0;
	};
}