#include "PlacingScreen.h"
#include <SFML/Graphics.hpp>
#include <SFML/Network/Packet.hpp>

#include "Eventsystem.h"
#include "imgui.h"
#include "Utils/json.hpp"

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
			ship_map[current_row][current_col] = static_cast<int8_t>(ship_index + 1);
		}
	}
	sf::RectangleShape sprite = create_ship_sprite(row, col, length, is_horizontal, offset);

	return { .coordinates = coordinates, .sprite = std::move(sprite), .segments_left = length, .destroyed = false };
}

void add_token_to_vertex_array(sf::VertexArray& vertex_array, int row, int col, int8_t type, sf::Vector2f offset)
{
	const sf::Vector2f cell_position{ (float)col * cell_size.x + offset.x ,(float)row * cell_size.y + offset.y };


	constexpr sf::Vector2f single_sprite_size = { 32.f,32.f };
	const sf::Vector2f top_left_tex_coord{ (float)type * single_sprite_size.x,0.f };
	const sf::Vector2f bottom_right_tex_coord = top_left_tex_coord + single_sprite_size;


	const sf::Vertex vertex1 = { .position = cell_position, .color = {sf::Color::White}, .texCoords = {top_left_tex_coord} };
	const sf::Vertex vertex2 = { .position = { cell_position.x + single_sprite_size.x,cell_position.y }, .color = {sf::Color::White},
		.texCoords = { bottom_right_tex_coord.x,top_left_tex_coord.y} };
	const sf::Vertex vertex3 = { .position = { cell_position.x,cell_position.y + single_sprite_size.y }, .color = {sf::Color::White},
		.texCoords = { top_left_tex_coord.x,bottom_right_tex_coord.y } };
	const sf::Vertex vertex4 = { .position = cell_position + single_sprite_size, .color = {sf::Color::White},
		.texCoords = {bottom_right_tex_coord} };


	vertex_array.append(vertex1);
	vertex_array.append(vertex2);
	vertex_array.append(vertex3);
	vertex_array.append(vertex2);
	vertex_array.append(vertex3);
	vertex_array.append(vertex4);
}

void change_token_from_vertex_array(sf::VertexArray& vertex_array, int row, int col, int8_t type, sf::Vector2f offset)
{
	const sf::Vector2f cell_position{ (float)col * cell_size.x + offset.x ,(float)row * cell_size.y + offset.y };
	constexpr sf::Vector2f single_sprite_size = { 32.f,32.f };
	const sf::Vector2f top_left_tex_coord{ (float)type * single_sprite_size.x,0.f };
	const sf::Vector2f bottom_right_tex_coord = top_left_tex_coord + single_sprite_size;

	for (int i = 0; i < vertex_array.getVertexCount(); i+=6)
	{
		if(vertex_array[i].position == cell_position)
		{
			vertex_array[i].texCoords = top_left_tex_coord;
			vertex_array[i+1].texCoords = { bottom_right_tex_coord.x,top_left_tex_coord.y };
			vertex_array[i+2].texCoords = { top_left_tex_coord.x,bottom_right_tex_coord.y };
			vertex_array[i+3].texCoords = { bottom_right_tex_coord.x,top_left_tex_coord.y };
			vertex_array[i+4].texCoords = { top_left_tex_coord.x,bottom_right_tex_coord.y };
			vertex_array[i+5].texCoords = bottom_right_tex_coord;
			break;
		}
	}
}

//PlacingScreen::PlacingScreen()
//{
//	m_ships_to_place = {1,2,1,1};
//
//	//nlohmann::json message;
//	//message["type"] = "game";
//	//message["kind"] = "place";
//	//message["row"] = m_row;
//	//message["col"] = m_col;
//	//message["length"] = m_length;
//	//message["is_horizontal"] = m_is_horizontal;
//	//sf::Packet packet;
//	//packet << message.dump();
//	//m_client->m_packets_to_be_sent.push_back(packet);
//
//}
//
//void PlacingScreen::update(std::shared_ptr<Eventsystem>& eventsystem, std::shared_ptr<LayerManager>& layer_manager,
//                           std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window, double deltatime)
//{
//	//ImGui::Begin("debug");
//
//
//	//if (eventsystem->get_key_action(sf::Keyboard::Key::Down) == Eventsystem::action_pressed || eventsystem->get_key_action(sf::Keyboard::Key::S) == Eventsystem::action_pressed)
//	//	m_row = (m_row + 1) % 10;
//	//if (eventsystem->get_key_action(sf::Keyboard::Key::Up) == Eventsystem::action_pressed
//	//	|| eventsystem->get_key_action(sf::Keyboard::Key::W) == Eventsystem::action_pressed)
//	//	m_row = m_row - 1 < 0 ? 9 : m_row - 1;
//
//	//if (eventsystem->get_key_action(sf::Keyboard::Key::Right) == Eventsystem::action_pressed
//	//	|| eventsystem->get_key_action(sf::Keyboard::Key::D) == Eventsystem::action_pressed)
//	//	m_col = (m_col + 1) % 10;
//	//if (eventsystem->get_key_action(sf::Keyboard::Key::Left) == Eventsystem::action_pressed
//	//	|| eventsystem->get_key_action(sf::Keyboard::Key::A) == Eventsystem::action_pressed)
//	//	m_col = m_col - 1 < 0 ? 9 : m_col - 1;
//
//	//if (eventsystem->get_key_action(sf::Keyboard::Key::R) == Eventsystem::action_pressed)
//	//	m_is_horizontal = !m_is_horizontal;
//	//if (eventsystem->get_key_action(sf::Keyboard::Key::E) == Eventsystem::action_pressed)
//	//	m_selected = m_selected + 1 > m_ships_to_place.size() ? 0 : m_selected + 1;
//	//if (eventsystem->get_key_action(sf::Keyboard::Key::Q) == Eventsystem::action_pressed)
//	//	m_selected = m_selected - 1 < 0 ? m_ships_to_place.size() - 1 : m_selected - 1;
//
//	//ImGui::SliderInt("row", &m_row, 0, 9);
//	//ImGui::SliderInt("col", &m_col, 0, 9);
//	//ImGui::Checkbox("is horizontal", &m_is_horizontal);
//	//if (ImGui::Button("place ship") || eventsystem->get_key_action(sf::Keyboard::Key::Enter) == Eventsystem::action_pressed)
//	//{
//
//	//	m_ship_place_function(m_row, m_col, m_ships_to_place[m_selected],m_is_horizontal);
//	//}
//
//	//ImGui::End();
//}
//
//void PlacingScreen::render(sf::RenderWindow& window)
//{
//
//}
//
//void PlacingScreen::on_close()
//{
//	
//}
//
//LayerID PlacingScreen::get_layer_id()
//{
//	return LayerID::game;
//}
