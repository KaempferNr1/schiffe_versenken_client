#pragma once
#include <memory>

#include "Layer.h"

class Client;
class Button;

namespace sf
{
	class RenderWindow;
}

class Soundsystem;
class LayerManager;
class Eventsystem;

class IdleLayer : public Layer
{
public:
	IdleLayer(std::shared_ptr<Client> client);

	void update(std::shared_ptr<Eventsystem>& eventsystem, std::shared_ptr<LayerManager>& layer_manager, std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window, double deltatime) override;

	void render(sf::RenderWindow& window) override;

	void on_close() override;

	[[nodiscard]] LayerID get_layer_id() override;
private:
	std::shared_ptr<Client> m_client;
	std::vector<std::unique_ptr<Button>> m_buttons;
	int m_selected = -1;
};

