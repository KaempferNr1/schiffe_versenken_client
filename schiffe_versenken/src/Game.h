#pragma once
#include <SFML/Graphics.hpp>
#include "Layer.h"
#include <SFML/Network/TcpSocket.hpp>

#include "Client.h"

class SkillSelector;
class Game : public Layer
{
public:
	Game(std::shared_ptr<Soundsystem>& soundsystem);

	void update(std::shared_ptr<Eventsystem>& eventsystem, std::shared_ptr<LayerManager>& layer_manager, std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window, double deltatime) override;

	void render(sf::RenderWindow& window) override;

	void on_close() override;

	[[nodiscard]] LayerID get_layer_id() override;
private:
	sf::View m_view;
	sf::Font m_font;
	std::shared_ptr<Client> m_client;
	std::shared_ptr<LayerManager> m_layer_manager;
	std::shared_ptr<Layer> m_current_layer;
};


