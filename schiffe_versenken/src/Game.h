#pragma once
#include <SFML/Graphics.hpp>
#include "Layer.h"
#include <SFML/Network/TcpSocket.hpp>

#include "Client.h"

class SkillSelector;
enum class status : uint8_t
{
	NO_CONNECTION,
	RECONNECT_SCREEN,
	IDLE,
	FINDING_GAME,
	PLACING_PHASE,
	SHOOTING_PHASE,
	GAME_DONE
};



class Game : public Layer
{
public:
	Game(std::shared_ptr<Soundsystem>& soundsystem);
	~Game() override;
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

	std::array<std::array<int8_t, 10>, 10> m_shots = {{false}};
	std::array<std::array<int8_t, 10>, 10> m_ship_map = {{0}};
	std::vector<Ship> m_ships;
	status m_status = status::IDLE;
	nlohmann::json m_server_info;

	friend class Client;
};


