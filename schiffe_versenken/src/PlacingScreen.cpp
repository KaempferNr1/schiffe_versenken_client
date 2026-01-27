#include "PlacingScreen.h"
#include <SFML/Graphics.hpp>

void Ship::render(sf::RenderWindow& window)
{
	window.draw(sprite);
}

bool validplacement(int row, int col, int length, bool is_horizontal, const std::array<std::array<int8_t, 10>, 10>& map)
{
	int amount_of_rows;
	int amount_of_cols;
	if (is_horizontal)
	{
		amount_of_cols = length;
		amount_of_rows = 1;
	}
	else
	{
		amount_of_cols = 1;
		amount_of_rows = length;
	}

	for (int offset_in_row = 0; offset_in_row < amount_of_rows; ++offset_in_row)
	{
		for (int offset_in_col = 0; offset_in_col < amount_of_cols; ++offset_in_col)
		{
			const int current_row = row + offset_in_row;
			const int current_col = col + offset_in_col;

			if (current_row < 0 || current_row > 9 ||
				current_col < 0 || current_col > 9)
			{
				return false;
			}

			if (map[current_row][current_col] != 0)
			{
				return false;
			}
		}
	}
	return true;
}

sf::Vector2f calculate_position(const int row, const int col, const sf::Vector2f offset)
{
	return { static_cast<float>(col) * cell_size.x + offset.x + cell_size.x, offset.y + cell_size.y + static_cast<float>(row) * cell_size.y };
}

sf::RectangleShape create_ship_sprite(const int row, const int col, const int length, const bool is_horizontal,
                                      const sf::Vector2f offset)
{
	sf::RectangleShape sprite;
	sf::IntRect texture_rect;

	if (is_horizontal)
	{
		texture_rect.size = { 32 + 31 * (length - 1),32 };
	}
	else
	{
		texture_rect.size = { 32,32 + 31 * (length - 1) };
	}
	if (length == 2)
	{
		if (is_horizontal)
		{
			texture_rect.position = { 244, 218 };
		}
		else
		{
			texture_rect.position = { 0,0 };
		}
	}
	else if (length == 3)
	{
		if (is_horizontal)
		{
			texture_rect.position = { 244, 256 };
		}
		else
		{
			texture_rect.position = { 38,0 };
		}
	}
	else if (length == 4)
	{
		if (is_horizontal)
		{
			texture_rect.position = { 244, 330 };
		}
		else
		{
			texture_rect.position = { 112,0 };
		}
	}
	else
	{
		if (is_horizontal)
		{
			texture_rect.position = { 244, 368 };
		}
		else
		{
			texture_rect.position = { 150,0 };
		}
	}


	sprite.setTextureRect(texture_rect);
	sprite.setSize(sf::Vector2f(texture_rect.size));
	sprite.setPosition(calculate_position(row,col,offset));
	return sprite;
}

Ship place_ship(int row, int col, int length, int is_horizontal, std::array<std::array<int8_t, 10>, 10>& ship_map,
	int ship_index, sf::Vector2f offset)
{
	int amount_of_rows;
	int amount_of_cols;
	if (is_horizontal)
	{
		amount_of_cols = length;
		amount_of_rows = 1;
	}
	else
	{
		amount_of_cols = 1;
		amount_of_rows = length;
	}
	std::vector<std::pair<int, int>> coordinates;
	for (int offset_in_col = 0; offset_in_col < amount_of_cols; ++offset_in_col)
	{
		for (int offset_in_row = 0; offset_in_row < amount_of_rows; ++offset_in_row)
		{
			const int current_row = row + offset_in_row;
			const int current_col = col + offset_in_col;
			coordinates.emplace_back(current_row, current_col);
			ship_map[current_row][current_col] = 1 + ship_index;
		}
	}
	sf::RectangleShape sprite = create_ship_sprite(row, col, length, is_horizontal, offset);

	return { .coordinates = coordinates, .sprite = std::move(sprite), .segments_left = length, .destroyed = false };
}

PlacingScreen::PlacingScreen()
{
	m_ships_to_place = {1,2,1,1};
}

void PlacingScreen::update(std::shared_ptr<Eventsystem>& eventsystem, std::shared_ptr<LayerManager>& layer_manager,
                           std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window, double deltatime)
{

}

void PlacingScreen::render(sf::RenderWindow& window)
{
	
}

void PlacingScreen::on_close()
{
	
}

LayerID PlacingScreen::get_layer_id()
{
	return LayerID::game;
}
