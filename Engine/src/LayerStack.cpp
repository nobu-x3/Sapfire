#include "engine/LayerStack.h"

LayerStack::LayerStack()
{
}

LayerStack::~LayerStack()
{
	for (auto layer : mLayers)
	{
		delete layer;
	}
}

void LayerStack::PushLayer(Layer *layer)
{
	mLayers.emplace(mLayers.begin() + mLayerInsertIndex, layer);
}

void LayerStack::PushOverlay(Layer *overlay)
{
	mLayers.emplace_back(overlay);
}

void LayerStack::PopLayer(Layer *layer)
{
	auto it = std::find(mLayers.begin(), mLayers.end(), layer);
	if (it != mLayers.end())
	{
		mLayers.erase(it);
		mLayerInsertIndex--;
	}
}

void LayerStack::PopOverlay(Layer *layer)
{
	auto it = std::find(mLayers.begin(), mLayers.end(), layer);
	if (it != mLayers.end())
	{
		mLayers.erase(it);
	}
}
