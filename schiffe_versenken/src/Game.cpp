#include "Game.h"

#include <fstream>
#include <glm/glm.hpp>

#include "Buttons/Button.h"
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
}

Game::~Game()
{
	LOG_INFO("game destructed");
}

void Game::update(std::shared_ptr<Eventsystem>& eventsystem, std::shared_ptr<LayerManager>& layer_manager, std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window, const double deltatime)
{

	if (!m_client)
	{
		m_client = std::make_shared<Client>(std::dynamic_pointer_cast<Game>(layer_manager->get_top()), sf::IpAddress::resolve(m_server_info["ip"]).value_or(sf::IpAddress::LocalHost), m_server_info["port"]);
		m_status = m_client->is_connected() ? status::IDLE : status::NO_CONNECTION;
		LOG_INFO("client connected: {}", m_client->is_connected());
	}
	std::optional<nlohmann::json> answer_optional = m_client->update();

	if (answer_optional.has_value())
	{
		nlohmann::json answer = answer_optional.value();
		const std::string type = answer["type"];
		if(type == "matchmaking started")
		{
			m_status = status::FINDING_GAME;
			m_ships.clear();
			for(int i = 0; i < 10; ++i)
			{
				for(int j = 0; j < 10; ++j)
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
		else if(type == "game_started")
		{
			m_status = status::SHOOTING_PHASE;
		}
		else if(type == "game done")
		{
			m_status = status::GAME_DONE;
		}
	}


	if (!m_client->is_connected() && m_status != status::RECONNECT_SCREEN)
	{
		m_layer_manager->clear();
		m_layer_manager->push_layer(std::make_shared<ReconnectScreen>(m_client, sf::IpAddress::resolve(m_server_info["ip"]).value_or(sf::IpAddress::LocalHost), m_server_info["port"]));
		m_status = status::RECONNECT_SCREEN;
	}
	else if(m_status == status::RECONNECT_SCREEN)
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
		else if(current_layer_is_reconnect_screen && m_layer_manager->is_empty())
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
	else if(m_status == status::IDLE)
	{
		if(m_current_layer && m_current_layer->get_layer_id() == LayerID::idle_layer && m_layer_manager->is_empty())
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
		if(m_layer_manager->is_empty())
		{
			m_layer_manager->push_layer(std::make_shared<IdleLayer>(m_client));
		}
	}
	else if(m_status == status::FINDING_GAME)
	{
		m_layer_manager->clear();
	}
	else if(m_status == status::GAME_DONE)
	{
		LOG_INFO("game done");
		m_status = status::IDLE;
	}
	else if(m_status == status::PLACING_PHASE)
	{
		ImGui::Begin("debug");
		static int row = 0;
		static int col = 0;
		static int length = 0;
		static bool is_horizontal = false;
		ImGui::SliderInt("row", &row, -1, 10);
		ImGui::SliderInt("col", &col, -1, 10);
		ImGui::SliderInt("length", &length, -1, 10);
		ImGui::Checkbox("is horizontal", &is_horizontal);
		if (ImGui::Button("place ship"))
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
		ImGui::Text("Ship board");
		for (int i = 0; i < 10; ++i)
		{
			ImGui::Text("%d %d %d %d %d %d %d %d %d %d", m_ship_map[i][0], m_ship_map[i][1], m_ship_map[i][2], m_ship_map[i][3], m_ship_map[i][4], m_ship_map[i][5], m_ship_map[i][6], m_ship_map[i][7], m_ship_map[i][8], m_ship_map[i][9]);
		}
		ImGui::End();
	}
	else
	{
		ImGui::Begin("debug");
		static int row = 0;
		static int col = 0;
		ImGui::SliderInt("row", &row, -1, 10);
		ImGui::SliderInt("col", &col, -1, 10);

		if (ImGui::Button("shoot"))
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
		ImGui::Text("Ship board");
		for (int i = 0; i < 10; ++i)
		{
			ImGui::Text("%d %d %d %d %d %d %d %d %d %d", m_ship_map[i][0], m_ship_map[i][1], m_ship_map[i][2], m_ship_map[i][3], m_ship_map[i][4], m_ship_map[i][5], m_ship_map[i][6], m_ship_map[i][7], m_ship_map[i][8], m_ship_map[i][9]);
		}

		ImGui::Text("hit board");
		for (int i = 0; i < 10; ++i)
		{
			ImGui::Text("%d %d %d %d %d %d %d %d %d %d", m_shots[i][0], m_shots[i][1], m_shots[i][2], m_shots[i][3], m_shots[i][4], m_shots[i][5], m_shots[i][6], m_shots[i][7], m_shots[i][8], m_shots[i][9]);
		}

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

