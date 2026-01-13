#pragma once
#include <memory>

#include "Implementations/Buttons/ButtonLayout.h"
#include "Implementations/Buttons/ButtonBehaviour.h"
#include "Eventsystem.h"

class Button
{
public:
	static constexpr sf::Vector2f large_button_size = { 400.f, 100.f };
	static constexpr sf::Vector2f medium_button_size = { 200.f, 50.f };
	static constexpr sf::Vector2f small_button_size = { 50.f,50.f };

	Button()
		: m_layout(std::make_shared<EmptyLayout>()), m_button_behaviour(std::make_shared<EmptyBehaviour>())
	{}

	void set_layout(const std::shared_ptr<ButtonLayout>& i_layout)
	{
		m_layout = i_layout;
	}

	void set_behaviour(const std::shared_ptr<ButtonBehaviour>& i_behaviour)
	{
		m_button_behaviour = i_behaviour;
	}

	[[nodiscard]] std::shared_ptr<ButtonLayout> get_layout() const
	{
		return m_layout;
	}

	[[nodiscard]] std::shared_ptr<ButtonBehaviour> get_behaviour() const
	{
		return m_button_behaviour;
	}


	[[nodiscard]] bool is_clicked() const
	{
		return m_layout->is_clicked();
	}

	[[nodiscard]] bool is_hovered() const
	{
		return m_layout->is_hovered();
	}

	void update(const std::shared_ptr<Eventsystem>& eventsystem) const
	{
		m_layout->update(eventsystem);
	}

	[[nodiscard]] bool on_click(std::shared_ptr<LayerManager>& layer_manager, std::shared_ptr<Soundsystem>& soundsystem,
		sf::RenderWindow& window) const
	{

		return m_button_behaviour->on_click(layer_manager, soundsystem, window);
	}

	void set_position(const sf::Vector2f& position) const
	{
		m_layout->set_position(position);
	}

	void render(sf::RenderWindow& window) const
	{
		m_layout->render(window);
	}

	void set_is_hovered(const bool hovered) const
	{
		m_layout->set_is_hovered(hovered);
	}

	void set_data(const std::string& i_string) const
	{
		m_layout->set_data(i_string);
	}

private:
	std::shared_ptr<ButtonLayout> m_layout;
	std::shared_ptr<ButtonBehaviour> m_button_behaviour;
};