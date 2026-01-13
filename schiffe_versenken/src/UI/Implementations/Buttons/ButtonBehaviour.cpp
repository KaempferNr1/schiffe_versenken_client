#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include "ButtonBehaviour.h"
#include "UI/Button.h"

#include "LayerManager.h"
#include "Game.h"
#include "LevelSelector.h"
#include "OptionsMenu.h"
#include "Utils/Log.h"
#include "Utils/Random.h"
#include "Utils/Soundsystem.h"
#include "Client.h"



bool GoBackTillLayer::on_click(std::shared_ptr<LayerManager>& layer_manager,
	[[maybe_unused]] std::shared_ptr<Soundsystem>& soundsystem,
	[[maybe_unused]] sf::RenderWindow& window)
{
	//soundsystem->play_sound("ui_sounds", 2);
	layer_manager->close_till_layer(m_layer_id);
	return true;
}

bool PopLayer::on_click(std::shared_ptr<LayerManager>& layer_manager,
	[[maybe_unused]] std::shared_ptr<Soundsystem>& soundsystem,
	[[maybe_unused]] sf::RenderWindow& window)
{
	//soundsystem->play_sound("ui_sounds", 2);
	layer_manager->pop_layer();
	return true;
}

bool AddLevelSelectLayer::on_click(std::shared_ptr<LayerManager>& layer_manager,
	[[maybe_unused]] std::shared_ptr<Soundsystem>& soundsystem,
	[[maybe_unused]] sf::RenderWindow& window)
{
	//soundsystem->play_sound("ui_sounds", Random::uint(0, 1));
	layer_manager->push_layer(std::make_shared<LevelSelector>());
	return true;
}

bool AddGameLayer::on_click(std::shared_ptr<LayerManager>& layer_manager,
	[[maybe_unused]] std::shared_ptr<Soundsystem>& soundsystem,
	[[maybe_unused]] sf::RenderWindow& window)
{
	//soundsystem->play_sound("ui_sounds", Random::uint(0, 1));
	layer_manager->push_layer(std::make_shared<Game>(soundsystem));
	return true;
}

bool AddOptionsMenu::on_click(std::shared_ptr<LayerManager>& layer_manager,
	[[maybe_unused]] std::shared_ptr<Soundsystem>& soundsystem,
	[[maybe_unused]] sf::RenderWindow& window)
{
	//soundsystem->play_sound("ui_sounds", Random::uint(0, 1));
	layer_manager->push_layer(std::make_shared<OptionsMenu>(soundsystem));
	return true;
}

bool IncrementVolume::on_click([[maybe_unused]] std::shared_ptr<LayerManager>& layer_manager,
	std::shared_ptr<Soundsystem>& soundsystem,
	[[maybe_unused]] sf::RenderWindow& window)
{
	//soundsystem->play_sound("ui_sounds", Random::uint(0, 1));
	soundsystem->increment_volume(m_volume, m_group_id);
	return false;
}

TryConnect::TryConnect(const std::shared_ptr<Client>& i_client,sf::IpAddress i_ip_address, unsigned short i_port) : m_client(i_client),
m_ip_address(i_ip_address), m_port(i_port)
{

}

bool TryConnect::on_click([[maybe_unused]] std::shared_ptr<LayerManager>& layer_manager, [[maybe_unused]] std::shared_ptr<Soundsystem>& soundsystem,
	[[maybe_unused]] sf::RenderWindow& window)
{
	m_client->reconnect(m_ip_address, m_port);
	return m_client->is_connected();
}

FindMatch::FindMatch(const std::shared_ptr<Client>& i_client) : m_client(i_client)
{
}

bool FindMatch::on_click([[maybe_unused]] std::shared_ptr<LayerManager>& layer_manager, [[maybe_unused]] std::shared_ptr<Soundsystem>& soundsystem,
	[[maybe_unused]] sf::RenderWindow& window)
{
	nlohmann::json message;
	message["type"] = "find";
	sf::Packet packet;
	packet << message.dump();
	m_client->m_packets_to_be_sent.push_back(packet);
	return true;
}
