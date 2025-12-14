#include "Game.h"
#include "Game.h"

#include <glm/glm.hpp>

#include "Buttons/Button.h"
#include "Eventsystem.h"
#include "imgui.h"
#include "LayerManager.h"
#include "PauseMenu.h"
#include "Utils/Log.h"
#include <SFML/Network.hpp>

#include "Client.h"
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
}

void Game::update(std::shared_ptr<Eventsystem>& eventsystem, std::shared_ptr<LayerManager>& layer_manager, std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window, const double deltatime)
{

	if(!m_client)
	{
		m_client = std::make_shared<Client>(std::dynamic_pointer_cast<Game>(layer_manager->get_top()), sf::IpAddress::LocalHost, 55001);
		LOG_INFO("client connected: {}", m_client->is_connected());
	}

	if (!m_client->is_connected())
	{
		if (m_layer_manager->is_empty()) 
		{
			m_layer_manager->push_layer(std::make_shared<ReconnectScreen>(m_client));
		}
	}
	else 
	{
		if(m_current_layer && m_current_layer->get_layer_id() == LayerID::reconnect_screen)
		{
			if (!m_layer_manager->is_empty() && m_layer_manager->get_top().get() == m_current_layer.get())
			{
				m_layer_manager->pop_layer();
			}
		}
		m_client->update();
		ImGui::Begin("debug");
		static int row = 0;
		static int col = 0;
		static int length = 0;
		static bool is_horizontal = false;
		ImGui::SliderInt("row",&row, -1, 10);
		ImGui::SliderInt("col", &col, -1, 10);
		ImGui::SliderInt("length", &length, -1, 10);
		ImGui::Checkbox("is horizontal", &is_horizontal);
		if(ImGui::Button("join matchmaking"))
		{
			nlohmann::json message;
			message["type"] = "find";
			sf::Packet packet;
			packet << message.dump();
			m_client->m_packets_to_be_sent.push_back(packet);
		}
		if(ImGui::Button("place ship"))
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
		if(ImGui::Button("shoot"))
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
		for(int i = 0; i < 10;++i)
		{
			ImGui::Text("%d %d %d %d %d %d %d %d %d %d", ship_map[i][0], ship_map[i][1], ship_map[i][2], ship_map[i][3], ship_map[i][4], ship_map[i][5], ship_map[i][6], ship_map[i][7], ship_map[i][8], ship_map[i][9]);
		}
		ImGui::Text("hit board");
		for (int i = 0; i < 10; ++i)
		{
			ImGui::Text("%d %d %d %d %d %d %d %d %d %d", shots[i][0], shots[i][1], shots[i][2], shots[i][3], shots[i][4], shots[i][5], shots[i][6], shots[i][7], shots[i][8], shots[i][9]);
		}

		ImGui::End();
	}


	if (!m_layer_manager->is_empty())
	{
		m_current_layer = m_layer_manager->get_top();
		m_current_layer->update(eventsystem, m_layer_manager, soundsystem, window, deltatime);

		if (m_layer_manager->is_empty() && !m_client->is_connected() && m_current_layer->get_layer_id() == LayerID::reconnect_screen)
		{
			layer_manager->pop_layer();
			LOG_INFO("nein danke");
		}
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
	if(m_current_layer)
	{
		m_current_layer->render(window);
	}
}

void Game::on_close()
{
	LOG_INFO("closed game");
}

LayerID Game::get_layer_id()
{
	return LayerID::game;
}

