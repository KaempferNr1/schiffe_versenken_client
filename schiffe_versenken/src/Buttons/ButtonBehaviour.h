#pragma once
#include <memory>

#include "Layer.h"
#include "Utils/Random.h"
#include "Utils/Soundsystem.h"


class Client;
#include "SFML/Network/IpAddress.hpp"

namespace sf
{
	class RenderWindow;
}
class LayerManager;
class Soundsystem;

class ButtonBehaviour
{
public:
	ButtonBehaviour() = default;
	ButtonBehaviour& operator=(const ButtonBehaviour&) = default;
	ButtonBehaviour(const ButtonBehaviour&) = default;
	ButtonBehaviour& operator=(ButtonBehaviour&&) noexcept = default;
	ButtonBehaviour(ButtonBehaviour&&) noexcept = default;
	virtual ~ButtonBehaviour() = default;
	[[nodiscard]] virtual bool on_click(std::shared_ptr<LayerManager>& layer_manager,
		std::shared_ptr<Soundsystem>& soundsystem,
		sf::RenderWindow& window
		) = 0;
};

class EmptyBehaviour : public ButtonBehaviour
{
public:
	[[nodiscard]] bool on_click([[maybe_unused]] std::shared_ptr<LayerManager>& layer_manager,
		[[maybe_unused]] std::shared_ptr<Soundsystem>& soundsystem,
		[[maybe_unused]] sf::RenderWindow& window) override
	{
		//soundsystem->play_sound("ui_sounds", Random::uint(0, 1));
		return false;
	}
};

class GoBackTillLayer : public ButtonBehaviour
{
public:
	GoBackTillLayer(const LayerID id): m_layer_id(id){}
	[[nodiscard]] bool on_click(std::shared_ptr<LayerManager>& layer_manager,
		std::shared_ptr<Soundsystem>& soundsystem, 
		sf::RenderWindow& window) override;
private:
	LayerID m_layer_id;
};

class PopLayer : public ButtonBehaviour
{
public:
	[[nodiscard]] bool on_click(std::shared_ptr<LayerManager>& layer_manager,
		std::shared_ptr<Soundsystem>& soundsystem, 
		sf::RenderWindow& window) override;
};

class AddLevelSelectLayer : public ButtonBehaviour
{
public:
	[[nodiscard]] bool on_click(std::shared_ptr<LayerManager>& layer_manager,
		std::shared_ptr<Soundsystem>& soundsystem,
		sf::RenderWindow& window) override;
};

class AddGameLayer : public ButtonBehaviour
{
public:
	[[nodiscard]] bool on_click(std::shared_ptr<LayerManager>& layer_manager, 
		std::shared_ptr<Soundsystem>& soundsystem,
		sf::RenderWindow& window) override;
private:
};

class AddOptionsMenu : public ButtonBehaviour
{
public:
	[[nodiscard]] bool on_click(std::shared_ptr<LayerManager>& layer_manager, std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window) override;
};

class IncrementVolume : public ButtonBehaviour
{
public:
	IncrementVolume(const std::string& i_group_id,float i_volume): m_group_id(i_group_id), m_volume(i_volume){}
	[[nodiscard]] bool on_click(std::shared_ptr<LayerManager>& layer_manager, std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window) override;
private:
	std::string m_group_id;
	float m_volume{5.f};
};

class TryConnect : public ButtonBehaviour
{
public:
	TryConnect(const std::shared_ptr<Client>& i_client, sf::IpAddress i_ip_address, unsigned short i_port);
	[[nodiscard]] bool on_click(std::shared_ptr<LayerManager>& layer_manager, std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window) override;
private:
	std::shared_ptr<Client> m_client;
	sf::IpAddress m_ip_address;
	unsigned short m_port;
};