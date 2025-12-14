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

	m_client = std::make_shared<Client>(sf::IpAddress::LocalHost, 55001);
//	int i = 0;

	LOG_INFO("client connected: {}", m_client->is_connected());
	m_layer_manager = std::make_shared<LayerManager>();
	
}

void Game::update(std::shared_ptr<Eventsystem>& eventsystem, std::shared_ptr<LayerManager>& layer_manager, std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window, const double deltatime)
{
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

