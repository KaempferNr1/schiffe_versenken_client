#include "Game.h"

#include <fstream>
#include <glm/glm.hpp>

#include "UI/Button.h"
#include "Eventsystem.h"
#include "imgui.h"
#include "LayerManager.h"
#include "PauseMenu.h"
#include "Utils/Log.h"
#include <SFML/Network.hpp>

#include "Client.h"
#include "IdleLayer.h"
#include "ReconnectScreen.h"
#include "Utils/json.hpp"
#include "../Resources/Images/Roboto-Regular.embed"

constexpr sf::Vector2f map_size{342.f,342.f};
constexpr float padding_between_maps = 20.f;
constexpr sf::Vector2f cell_size{ 31.f,31.f };
constexpr sf::Vector2f player_map_offset{50.f,50.f};
constexpr sf::Vector2f opponent_map_offset{ player_map_offset.x+map_size.x+padding_between_maps,player_map_offset.y};

Ship place_ships(int row, int col, int length, int is_horizontal, std::array<std::array<int8_t, 10>, 10>& ship_map, int ship_index)
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
	sprite.setPosition({ (float)col * cell_size.x + player_map_offset.x + cell_size.x, player_map_offset.y + cell_size.y + (float)row * cell_size.y });
	return { coordinates, std::move(sprite), length, false };
}


Game::Game(std::shared_ptr<Soundsystem>& soundsystem)
{
	if (!m_font.openFromMemory(g_RobotoRegular, sizeof(g_RobotoRegular)))
	{
		LOG_ERROR("failed to load font");
	}
	//if (!texture3->loadFromFile("Resources/Images/")) LOG_ERROR("failed to load structure 3");

	//m_structures.emplace_back(, sf::Vector2f(100.f, 0.f));

	/*for (int i = 0; i < 2; ++i)
	{
		sf::Sprite tx;

		tx.setTexture(*m_structure_textures[i]);
		tx.setPosition(structure_positions[i]);
		m_structures.push_back(tx);
	}*/
	m_layer_manager = std::make_shared<LayerManager>();
	std::ifstream f("server.json");
	m_server_info = nlohmann::json::parse(f);
	if (!m_player_background_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "oceangrid_final.png"))LOG_ERROR("Failed to load");
	if (!m_opponent_background_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "radargrid_final.png"))LOG_ERROR("Failed to load");
	if (!m_tokens_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "Tokens.png"))LOG_ERROR("Failed to load");
	if (!m_ship_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "BattleShipSheet_final.png"))LOG_ERROR("Failed to load");

	m_tokens_vertex_array.setPrimitiveType(sf::PrimitiveType::Triangles);

}

Game::~Game()
{
	LOG_INFO("game destructed");
}
void add_token_to_vertex_array(sf::VertexArray& vertex_array, int row, int col, int8_t type, sf::Vector2f offset)
{
	const sf::Vector2f cell_position{ (float)col * cell_size.x + offset.x ,(float)row * cell_size.y + offset.y};


	constexpr sf::Vector2f single_sprite_size = {32.f,32.f};
	const sf::Vector2f top_left_tex_coord{(float)type * single_sprite_size.x,0.f};
	const sf::Vector2f bottom_right_tex_coord = top_left_tex_coord + single_sprite_size;


	const sf::Vertex vertex1 = {.position = cell_position, .color = {sf::Color::White}, .texCoords = {top_left_tex_coord} };
	const sf::Vertex vertex2 = {.position = { cell_position.x + single_sprite_size.x,cell_position.y }, .color = {sf::Color::White},
		.texCoords = { bottom_right_tex_coord.x,top_left_tex_coord.y} };
	const sf::Vertex vertex3 = {.position = { cell_position.x,cell_position.y + single_sprite_size.y }, .color = {sf::Color::White},
		.texCoords = { top_left_tex_coord.x,bottom_right_tex_coord.y } };
	const sf::Vertex vertex4 = {.position = cell_position + single_sprite_size, .color = {sf::Color::White},
		.texCoords = {bottom_right_tex_coord} };


	vertex_array.append(vertex1);
	vertex_array.append(vertex2);
	vertex_array.append(vertex3);
	vertex_array.append(vertex2);
	vertex_array.append(vertex3);
	vertex_array.append(vertex4);

}



void Game::update(std::shared_ptr<Eventsystem>& eventsystem, std::shared_ptr<LayerManager>& layer_manager, std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window, const double deltatime)
{
	static std::string status;
	if (!m_client)
	{
		m_client = std::make_shared<Client>(sf::IpAddress::resolve(m_server_info["ip"]).value_or(sf::IpAddress::LocalHost), m_server_info["port"]);
		m_status = m_client->is_connected() ? status::IDLE : status::NO_CONNECTION;
		LOG_INFO("client connected: {}", m_client->is_connected());
	}
	nlohmann::json answer = m_client->update();

	if (!answer.empty())
	{
		const std::string type = answer["type"];
		if (type == "matchmaking started")
		{
			m_status = status::FINDING_GAME;
			m_ships.clear();
			m_tokens_vertex_array.clear();
			for (int i = 0; i < 10; ++i)
			{
				for (int j = 0; j < 10; ++j)
				{
					m_ship_map[i][j] = 0;
					m_shots[i][j] = 0;
				}
			}
		}
		if (type == "found")
		{
			m_status = status::PLACING_PHASE;
		}
		else if (type == "game_started")
		{
			m_status = status::SHOOTING_PHASE;
			if (answer["turn"] == "enemy")
				status = "enemy turn";
			else
				status = "your turn";
		}
		else if (type == "game done")
		{
			LOG_INFO("game done");
			m_status = status::GAME_DONE;
			std::vector<std::vector<int8_t>> temp_map = answer["board"];
			std::array<std::vector<std::pair<int, int>>,5> coordinates;
			std::array<bool,5> horizontal;
			for(int i = 0; i < 10; i++)
			{
				for(int j = 0; j < 10;j++)
				{
					const int8_t cell = temp_map[i][j];
					if(cell != 0)
					{
						if(!coordinates[cell - 1].empty())
						{
							horizontal[cell - 1] = i == coordinates[cell - 1].back().first;
						}
						coordinates[cell - 1].emplace_back(i, j);
					}

				}
			}

			for (int i = 0; i < coordinates.size();++i)
			{
				sf::RectangleShape sprite;
				sf::IntRect texture_rect;
				const bool is_horizontal = horizontal[i];
				const int length = coordinates[i].size();
				if (length == 0)
					continue;
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
				sprite.setPosition({ (float)coordinates[i].front().second * cell_size.x + opponent_map_offset.x + cell_size.x, +opponent_map_offset.y + cell_size.y + (float)coordinates[i].front().first * cell_size.y });
				sprite.setTexture(&m_ship_texture);
				m_ships.emplace_back(coordinates[i], std::move(sprite));
			}
		}
		else if (type == "shot")
		{
			

			if (answer["kind"] != "receive")
			{
				status = "enemy turn";

				const int row = answer["row"];
				const int col = answer["col"];
				m_shots[row][col] = answer["kind"] == "miss" ? 1 : 2;
				add_token_to_vertex_array(m_tokens_vertex_array, row, col, m_shots[row][col] - 1,{opponent_map_offset + cell_size});
			}
			else
			{
				status = "your turn";

				const int row = answer["row"];
				const int col = answer["col"];
				int8_t& cell = m_ship_map[row][col];

				if (cell > 0)
				{
					if (--m_ships[cell - 1].segments_left == 0)
					{
						m_ships[cell - 1].destroyed = true;
					}
					cell = -1;
				}
				else
				{
					cell = -2;
				}
				add_token_to_vertex_array(m_tokens_vertex_array, row, col, cell + 4, {player_map_offset + cell_size});

			}
		}
		else if (type == "success")
		{
			const int row = answer["row"];
			const int col = answer["col"];
			const int length = answer["length"];
			const int is_horizontal = answer["is_horizontal"];
			m_ships.emplace_back(place_ships(row, col, length, is_horizontal,m_ship_map,m_ships.size())).sprite.setTexture(&m_ship_texture);
		}
	}


	if (!m_client->is_connected() && m_status != status::RECONNECT_SCREEN && m_status != status::GAME_DONE)
	{
		m_layer_manager->clear();
		m_layer_manager->push_layer(std::make_shared<ReconnectScreen>(m_client, sf::IpAddress::resolve(m_server_info["ip"]).value_or(sf::IpAddress::LocalHost), m_server_info["port"]));
		m_status = status::RECONNECT_SCREEN;
	}
	else if (m_status == status::RECONNECT_SCREEN)
	{
		const bool current_layer_is_reconnect_screen = m_current_layer && m_current_layer->get_layer_id() == LayerID::reconnect_screen;

		if (m_client->is_connected() && current_layer_is_reconnect_screen)
		{
			if (!m_layer_manager->is_empty() && m_layer_manager->get_top().get() == m_current_layer.get())
			{
				m_layer_manager->pop_layer();
			}
			m_status = status::IDLE;
		}
		else if (current_layer_is_reconnect_screen && m_layer_manager->is_empty())
		{
			if (this == layer_manager->get_top().get()) {
				layer_manager->close_till_layer(LayerID::level_selector);
				LOG_INFO("connection cancelled");
				return;
			}
			else
			{
				m_layer_manager->push_layer(m_current_layer);
			}

		}
	}
	else if (m_status == status::IDLE)
	{
		if (m_current_layer && m_current_layer->get_layer_id() == LayerID::idle_layer && m_layer_manager->is_empty())
		{
			if (this == layer_manager->get_top().get()) {
				layer_manager->close_till_layer(LayerID::level_selector);
				LOG_INFO("cancelled");
				return;
			}
			else
			{
				m_layer_manager->push_layer(m_current_layer);
			}
		}
		if (m_layer_manager->is_empty())
		{
			m_layer_manager->push_layer(std::make_shared<IdleLayer>(m_client));
		}
	}
	else if (m_status == status::FINDING_GAME)
	{
		if(!m_layer_manager->is_empty())
			m_layer_manager->clear();
	}
	else if (m_status == status::GAME_DONE)
	{
		if(eventsystem->get_key_action(sf::Keyboard::Key::Enter) == Eventsystem::action_pressed)
			m_status = status::IDLE;
	}
	else if (m_status == status::PLACING_PHASE)
	{
		ImGui::Begin("debug");
		static int row = 0;
		static int col = 0;
		static int length = 2;
		static bool is_horizontal = false;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Down) == Eventsystem::action_pressed || eventsystem->get_key_action(sf::Keyboard::Key::S) == Eventsystem::action_pressed)
			row = (row + 1) % 10;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Up) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::W) == Eventsystem::action_pressed)
			row = row - 1 < 0 ? 9 : row - 1;

		if (eventsystem->get_key_action(sf::Keyboard::Key::Right) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::D) == Eventsystem::action_pressed)
			col = (col + 1) % 10;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Left) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::A) == Eventsystem::action_pressed)
			col = col - 1 < 0 ? 9 : col - 1;

		if (eventsystem->get_key_action(sf::Keyboard::Key::R) == Eventsystem::action_pressed)
			is_horizontal = !is_horizontal;
		if (eventsystem->get_key_action(sf::Keyboard::Key::E) == Eventsystem::action_pressed)
			length = length + 1 > 5 ? 2 : length + 1;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Q) == Eventsystem::action_pressed)
			length = length - 1 < 2 ? 5 : length - 1;
		
		ImGui::SliderInt("row", &row, 0, 9);
		ImGui::SliderInt("col", &col, 0, 9);
		ImGui::SliderInt("length", &length, 2, 5);
		ImGui::Checkbox("is horizontal", &is_horizontal);
		if (ImGui::Button("place ship") || eventsystem->get_key_action(sf::Keyboard::Key::Enter) == Eventsystem::action_pressed)
		{
			nlohmann::json message;
			message["type"] = "game";
			message["kind"] = "place";
			message["row"] = row;
			message["col"] = col;
			message["length"] = length;
			message["is_horizontal"] = is_horizontal;
			sf::Packet packet;
			packet << message.dump();
			m_client->m_packets_to_be_sent.push_back(packet);
		}
		//ImGui::Text("Ship board");
		//for (int i = 0; i < 10; ++i)
		//{
		//	ImGui::Text("%d %d %d %d %d %d %d %d %d %d", m_ship_map[i][0], m_ship_map[i][1], m_ship_map[i][2], m_ship_map[i][3], m_ship_map[i][4], m_ship_map[i][5], m_ship_map[i][6], m_ship_map[i][7], m_ship_map[i][8], m_ship_map[i][9]);
		//}
		ImGui::End();
	}
	else
	{
		ImGui::Begin("debug");
		ImGui::Text("%s", status.c_str());
		static int row = 0;
		static int col = 0;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Down) == Eventsystem::action_pressed 
			|| eventsystem->get_key_action(sf::Keyboard::Key::S) == Eventsystem::action_pressed)
			row = (row + 1) % 10;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Up) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::W) == Eventsystem::action_pressed)
			row = row - 1 < 0 ? 9 : row - 1;

		if (eventsystem->get_key_action(sf::Keyboard::Key::Right) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::D) == Eventsystem::action_pressed)
			col = (col + 1) % 10;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Left) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::A) == Eventsystem::action_pressed)
			col = col - 1 < 0 ? 9 : col - 1;

		ImGui::SliderInt("row", &row, 0, 9);
		ImGui::SliderInt("col", &col, 0, 9);

		if (ImGui::Button("shoot") || eventsystem->get_key_action(sf::Keyboard::Key::Enter) == Eventsystem::action_pressed)
		{
			nlohmann::json message;
			message["type"] = "game";
			message["kind"] = "move";
			message["row"] = row;
			message["col"] = col;
			sf::Packet packet;
			packet << message.dump();
			m_client->m_packets_to_be_sent.push_back(packet);
		}
		//ImGui::Text("Ship board");
		//for (int i = 0; i < 10; ++i)
		//{
		//	ImGui::Text("%d %d %d %d %d %d %d %d %d %d", m_ship_map[i][0], m_ship_map[i][1], m_ship_map[i][2], m_ship_map[i][3], m_ship_map[i][4], m_ship_map[i][5], m_ship_map[i][6], m_ship_map[i][7], m_ship_map[i][8], m_ship_map[i][9]);
		//}

		//ImGui::Text("hit board");
		//for (int i = 0; i < 10; ++i)
		//{
		//	ImGui::Text("%d %d %d %d %d %d %d %d %d %d", m_shots[i][0], m_shots[i][1], m_shots[i][2], m_shots[i][3], m_shots[i][4], m_shots[i][5], m_shots[i][6], m_shots[i][7], m_shots[i][8], m_shots[i][9]);
		//}

		ImGui::End();

	}


	if (!m_layer_manager->is_empty())
	{
		m_current_layer = m_layer_manager->get_top();
		m_current_layer->update(eventsystem, m_layer_manager, soundsystem, window, deltatime);
	}
	else
	{
		m_current_layer.reset();
	}







	if (this == layer_manager->get_top().get() && (eventsystem->get_key_action(sf::Keyboard::Key::Escape) == Eventsystem::action_pressed /*|| !eventsystem->has_focus()*/))
	{
		layer_manager->push_layer(std::make_shared<PauseMenu>(layer_manager->get_top()));
		return;
	}
}

void Game::render(sf::RenderWindow& window)
{
	if (m_current_layer)
	{
		m_current_layer->render(window);
	}

	if (m_status != status::GAME_DONE && m_status != status::SHOOTING_PHASE && m_status != status::PLACING_PHASE)
		return;

	sf::RectangleShape rect;
	rect.setPosition(player_map_offset);
	rect.setTexture(&m_player_background_texture);
	rect.setSize(map_size);
	window.draw(rect);

	if (m_status != status::PLACING_PHASE)
	{
		rect.setPosition(opponent_map_offset);
		rect.setTexture(&m_opponent_background_texture);
		window.draw(rect);
	}

	for (Ship& ship : m_ships)
	{
		ship.render(window);
	}
	if (m_status == status::PLACING_PHASE)
		return;
	sf::RenderStates states = sf::RenderStates::Default;
	states.texture = &m_tokens_texture;
	window.draw(m_tokens_vertex_array, states);

}

void Game::on_close()
{
	m_layer_manager.reset();
	m_client.reset();
	m_current_layer.reset();
	LOG_INFO("closed game");
}

LayerID Game::get_layer_id()
{
	return LayerID::game;
}