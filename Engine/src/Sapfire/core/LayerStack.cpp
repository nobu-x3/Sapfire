#include "engpch.h"
#include "LayerStack.h"

namespace Sapfire
{
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

	void LayerStack::push_layer(Layer* layer)
	{
		mLayers.emplace(mLayers.begin() + mLayerInsertIndex, layer);
	}

	void LayerStack::push_overlay(Layer* overlay)
	{
		mLayers.emplace_back(overlay);
		mLayerInsertIndex++;
	}

	void LayerStack::pop_layer(Layer* layer)
	{
		auto it = std::find(mLayers.begin(), mLayers.end(), layer);
		if (it != mLayers.end())
		{
			mLayers.erase(it);
			mLayerInsertIndex--;
		}
	}

	void LayerStack::pop_overlay(Layer* layer)
	{
		auto it = std::find(mLayers.begin(), mLayers.end(), layer);
		if (it != mLayers.end())
		{
			mLayers.erase(it);
		}
	}
}