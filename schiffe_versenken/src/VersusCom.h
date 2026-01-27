#pragma once
#include <array>
#include <future>

#include "Game.h"
#include "PlacingScreen.h"
#include "Layer.h"
#include "Utils/Random.h"

class Computer
{
public:
	std::array<std::array<int8_t, 10>, 10> m_shots = { {0} };
	std::array<std::array<int8_t, 10>, 10> m_ship_map = { {0} };
	std::array<Ship, 5> m_ships;
	std::array<int, 4> shipsleft{ 1,2,1,1 };
private:
	std::vector<std::vector<double>> m_probabilities = std::vector<std::vector<double>>(10, std::vector<double>(10, 1));

	void place_ships();

public:
	Computer();

	void change_probs(int col, int row, double change, bool do_smarter_check);

	std::pair<int,int> make_move();
};

class VersusCom : public Layer
{
	std::unique_ptr<Computer> m_computer;
	std::vector<Ship> m_ships;

	sf::Texture m_player_background_texture;
	sf::Texture m_opponent_background_texture;
	sf::Texture m_tokens_texture;
	sf::Texture m_ship_texture;

	sf::VertexArray m_tokens_vertex_array;

	status m_status = status::PLACING_PHASE;
	std::array<std::array<int8_t, 10>, 10> m_shots = { {0} };
	std::array<std::array<int8_t, 10>, 10> m_ship_map = { {0} };
	std::array<int, 4> ships_left{1,2,1,1};
	int m_row{ 0 };
	int m_col{ 0 };
	int m_length{ 2 };
	bool m_is_horizontal{ false };

	int m_player_ships_remaining = 5;
	int m_computer_ships_remaining = 5;
public:
	VersusCom();
	void update([[maybe_unused]] std::shared_ptr<Eventsystem>& eventsystem, [[maybe_unused]] std::shared_ptr<LayerManager>& layer_manager,
		[[maybe_unused]] std::shared_ptr<Soundsystem>& soundsystem, [[maybe_unused]] sf::RenderWindow& window, [[maybe_unused]] double deltatime) override;

	void render([[maybe_unused]] sf::RenderWindow& window) override;

	void on_close() override;
	[[nodiscard]] LayerID get_layer_id() override;
};