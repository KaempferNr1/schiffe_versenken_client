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
#include "PlacingScreen.h"
#include "ReconnectScreen.h"
#include "Utils/json.hpp"
#include "../Resources/Images/Roboto-Regular.embed"



Game::Game([[maybe_unused]] std::shared_ptr<Soundsystem>& soundsystem)
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
	if (!m_tokens_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "Tokens 2.png"))LOG_ERROR("Failed to load");
	if (!m_ship_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "BattleShipSheet_final.png"))LOG_ERROR("Failed to load");

	m_tokens_vertex_array.setPrimitiveType(sf::PrimitiveType::Triangles);

}

Game::~Game()
{
	LOG_INFO("game destructed");
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
			int8_t horizontal = 0;
			for(int i = 0; i < 10; i++)
			{
				for(int j = 0; j < 10;j++)
				{
					const int8_t cell = temp_map[i][j];
					if(cell != 0)
					{
						if(!coordinates[cell - 1].empty())
						{
							horizontal |= (i == coordinates[cell - 1].back().first) << (cell - 1);
						}
						coordinates[cell - 1].emplace_back(i, j);
					}

				}
			}

			for (int i = 0; i < coordinates.size();++i)
			{
				const bool is_horizontal = (horizontal >> i) & 1;
				const int length = static_cast<int>(coordinates[i].size());
				if (length == 0)
					continue;
				const int row = coordinates[i].front().first;
				const int col = coordinates[i].front().second;
				sf::RectangleShape sprite = create_ship_sprite(row, col , length, is_horizontal, opponent_map_offset);
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
				if (answer["kind"] == "destroyed")
					add_token_to_vertex_array(m_tokens_vertex_array, row, col, 4, { opponent_map_offset + cell_size });
				else
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
			m_ships.emplace_back(place_ship(row, col, length, is_horizontal,m_ship_map,static_cast<int>(m_ships.size()),player_map_offset)).sprite.setTexture(&m_ship_texture);
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
		if (eventsystem->get_key_action(sf::Keyboard::Key::Down) == Eventsystem::action_pressed || eventsystem->get_key_action(sf::Keyboard::Key::S) == Eventsystem::action_pressed)
			m_row = (m_row + 1) % 10;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Up) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::W) == Eventsystem::action_pressed)
			m_row = m_row - 1 < 0 ? 9 : m_row - 1;

		if (eventsystem->get_key_action(sf::Keyboard::Key::Right) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::D) == Eventsystem::action_pressed)
			m_col = (m_col + 1) % 10;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Left) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::A) == Eventsystem::action_pressed)
			m_col = m_col - 1 < 0 ? 9 : m_col - 1;

		if (eventsystem->get_key_action(sf::Keyboard::Key::R) == Eventsystem::action_pressed)
			m_is_horizontal = !m_is_horizontal;
		if (eventsystem->get_key_action(sf::Keyboard::Key::E) == Eventsystem::action_pressed)
			m_length = m_length + 1 > 5 ? 2 : m_length + 1;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Q) == Eventsystem::action_pressed)
			m_length = m_length - 1 < 2 ? 5 : m_length - 1;
		
		ImGui::SliderInt("row", &m_row, 0, 9);
		ImGui::SliderInt("col", &m_col, 0, 9);
		ImGui::SliderInt("length", &m_length, 2, 5);
		ImGui::Checkbox("is horizontal", &m_is_horizontal);
		if (ImGui::Button("place ship") || eventsystem->get_key_action(sf::Keyboard::Key::Enter) == Eventsystem::action_pressed)
		{
			nlohmann::json message;
			message["type"] = "game";
			message["kind"] = "place";
			message["row"] = m_row;
			message["col"] = m_col;
			message["length"] = m_length;
			message["is_horizontal"] = m_is_horizontal;
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
		if (eventsystem->get_key_action(sf::Keyboard::Key::Down) == Eventsystem::action_pressed || eventsystem->get_key_action(sf::Keyboard::Key::S) == Eventsystem::action_pressed)
			m_row = (m_row + 1) % 10;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Up) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::W) == Eventsystem::action_pressed)
			m_row = m_row - 1 < 0 ? 9 : m_row - 1;

		if (eventsystem->get_key_action(sf::Keyboard::Key::Right) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::D) == Eventsystem::action_pressed)
			m_col = (m_col + 1) % 10;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Left) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::A) == Eventsystem::action_pressed)
			m_col = m_col - 1 < 0 ? 9 : m_col - 1;

		ImGui::SliderInt("row", &m_row, 0, 9);
		ImGui::SliderInt("col", &m_col, 0, 9);

		if (ImGui::Button("shoot") || eventsystem->get_key_action(sf::Keyboard::Key::Enter) == Eventsystem::action_pressed)
		{
			nlohmann::json message;
			message["type"] = "game";
			message["kind"] = "move";
			message["row"] = m_row;
			message["col"] = m_col;
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
	{

		sf::RectangleShape selection = create_ship_sprite(m_row, m_col, m_length, m_is_horizontal, player_map_offset);
		selection.setTexture(&m_ship_texture);
		selection.setFillColor({ 255,255,255,127 });
		window.draw(selection);
		return;
	}
	sf::RenderStates states = sf::RenderStates::Default;
	states.texture = &m_tokens_texture;
	window.draw(m_tokens_vertex_array, states);
	
	if (m_status == status::SHOOTING_PHASE) 
	{
		sf::RectangleShape selection{ {32.f,32.f} };
		selection.setPosition(calculate_position(m_row, m_col, opponent_map_offset));
		selection.setFillColor({ 127,127,127,127 });
		window.draw(selection);
	}
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