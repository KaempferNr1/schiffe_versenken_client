#pragma once
#include <memory>
#include <stack>

#include "Layer.h"

class Layer;

class LayerManager
{
public:
	[[nodiscard]] std::shared_ptr<Layer>& get_top();

	void push_layer(const std::shared_ptr<Layer>& layer);

	void pop_layer();

	[[nodiscard]] bool is_empty() const;

	void close_till_layer(LayerID target_layer_id);
	void clear() { m_layers.clear(); }

private:
	std::deque<std::shared_ptr<Layer>> m_layers;
};

