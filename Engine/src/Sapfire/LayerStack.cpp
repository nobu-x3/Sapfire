#include "engpch.h"
#include "LayerStack.h"

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
	layer->OnAttach();
}

void LayerStack::PushOverlay(Layer *overlay)
{
	mLayers.emplace_back(overlay);
	overlay->OnAttach();
	mLayerInsertIndex++;
}

void LayerStack::PopLayer(Layer *layer)
{
	auto it = std::find(mLayers.begin(), mLayers.end(), layer);
	if (it != mLayers.end())
	{
		mLayers.erase(it);
		mLayerInsertIndex--;
	}
	layer->OnDetach();
}

void LayerStack::PopOverlay(Layer *layer)
{
	auto it = std::find(mLayers.begin(), mLayers.end(), layer);
	if (it != mLayers.end())
	{
		mLayers.erase(it);
	}
	layer->OnDetach();
}
