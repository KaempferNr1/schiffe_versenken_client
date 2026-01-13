#include "ReconnectScreen.h"

#include <SFML/Network/IpAddress.hpp>

#include "Eventsystem.h"
#include "Utils/Soundsystem.h"
#include "LayerManager.h"
#include "UI/Button.h"


ReconnectScreen::ReconnectScreen(std::shared_ptr<Client> client, sf::IpAddress ip, unsigned short port)
{
	m_buttons.emplace_back(std::make_unique<Button>());
	m_buttons.emplace_back(std::make_unique<Button>());

	m_buttons[0]->set_layout(std::make_shared<TextLayout>("verbinden", sf::Vector2f{ 260.f,145.f }, Button::large_button_size));
	m_buttons[0]->set_behaviour(std::make_shared<TryConnect>(client,ip, port));

	m_buttons[1]->set_layout(std::make_shared<TextLayout>("schließen", sf::Vector2f{ 260.f,285.f }, Button::large_button_size));
	m_buttons[1]->set_behaviour(std::make_shared<PopLayer>());
}
void ReconnectScreen::update(std::shared_ptr<Eventsystem>& eventsystem, std::shared_ptr<LayerManager>& layer_manager,
	std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window, [[maybe_unused]] double deltatime)
{
	const sf::Vector2f mouse_pos = eventsystem->get_mouse_position();

	constexpr float padding = 20.0f;
	const float total_height = static_cast<float>(m_buttons.size()) * Button::large_button_size.y + (static_cast<float>(m_buttons.size()) - 1.f) * padding;
	const float start_y = (static_cast<float>(eventsystem->get_window_size().y) - total_height) / 2.f;
	const float start_x = static_cast<float>(eventsystem->get_window_size().x) / 2.f - Button::large_button_size.x / 2;

	for (uint8_t i = 0; i < m_buttons.size(); ++i)
	{
		m_buttons[i]->set_position({ start_x ,start_y + static_cast<float>(i) * (Button::large_button_size.y + padding) });
		m_buttons[i]->update(eventsystem);
		if (!m_buttons[i]->is_clicked())
			continue;

		if (m_buttons[i]->on_click(layer_manager, soundsystem, window))
		{
			m_selected = -1;
			return;
		}
	}


	if (eventsystem->get_key_action(sf::Keyboard::Key::Down) == Eventsystem::action_pressed)
		m_selected = m_selected + 1 == static_cast<int>(m_buttons.size()) ? 0 : m_selected + 1;

	if (eventsystem->get_key_action(sf::Keyboard::Key::Up) == Eventsystem::action_pressed)
		m_selected = m_selected - 1 < 0 ? static_cast<int>(m_buttons.size()) - 1 : m_selected - 1;

	if (eventsystem->get_mouse_button_action(sf::Mouse::Button::Left) == Eventsystem::action_pressed)
		m_selected = -1;

	if (m_selected == -1)
		return;

	m_buttons[m_selected]->set_is_hovered(true);

	if (eventsystem->get_key_action(sf::Keyboard::Key::Enter) == Eventsystem::action_released)
	{
		if (m_buttons[m_selected]->on_click(layer_manager, soundsystem, window))
			m_selected = -1;

		return;
	}
}

void ReconnectScreen::render(sf::RenderWindow& window)
{
	for (const auto& button : m_buttons)
	{
		button->render(window);
	}
}

void ReconnectScreen::on_close()
{

}

LayerID ReconnectScreen::get_layer_id()
{
	return LayerID::reconnect_screen;
}
