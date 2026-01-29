#pragma once
#include <functional>
#include <SFML/Graphics/RectangleShape.hpp>

#include "Layer.h"

constexpr sf::Vector2f map_size{ 342.f,342.f };
constexpr float padding_between_maps = 20.f;
constexpr sf::Vector2f cell_size{ 31.f,31.f };
constexpr sf::Vector2f player_map_offset{ 8.f,8.f };
constexpr sf::Vector2f opponent_map_offset{ player_map_offset.x + map_size.x + padding_between_maps,player_map_offset.y };

struct Ship
{
	std::vector<std::pair<int, int>> coordinates;
	sf::RectangleShape sprite;
	int segments_left{ 1 };
	bool destroyed{ false };

	void render(sf::RenderWindow&);
};



bool validplacement(int row, int col, int length, bool is_horizontal, const std::array<std::array<int8_t, 10>, 10>& map);

sf::Vector2f calculate_position(int row, int col, sf::Vector2f offset);

sf::RectangleShape create_ship_sprite(const int row, const int col, const int length, const bool is_horizontal, const sf::Vector2f offset);

Ship place_ship(int row, int col, int length, int is_horizontal, std::array<std::array<int8_t, 10>, 10>& ship_map, int ship_index, sf::Vector2f offset);

void add_token_to_vertex_array(sf::VertexArray& vertex_array, int row, int col, int8_t type, sf::Vector2f offset);

void change_token_from_vertex_array(sf::VertexArray& vertex_array, int row, int col, int8_t type, sf::Vector2f offset);

//class PlacingScreen : public Layer
//{
//private:
//	sf::Texture* m_ship_texture;
//	sf::Texture* m_background_texture;
//	std::vector<int> m_ships_to_place;
//	int m_row{0};
//	int m_col{0};
//	int m_selected{0};
//	bool m_is_horizontal{0};
//	bool m_online;
//	std::function<void(int,int,int,bool)> m_ship_place_function;
//public:
//	PlacingScreen();
//	void update([[maybe_unused]] std::shared_ptr<Eventsystem>& eventsystem, [[maybe_unused]] std::shared_ptr<LayerManager>& layer_manager,
//	            [[maybe_unused]] std::shared_ptr<Soundsystem>& soundsystem, [[maybe_unused]] sf::RenderWindow& window, [[maybe_unused]] double deltatime) override;
//
//	void render([[maybe_unused]] sf::RenderWindow& window) override;
//
//	void on_close() override;
//	[[nodiscard]] LayerID get_layer_id() override;
//};
//
