#pragma once
#include <algorithm>
#include <memory>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "Eventsystem.h"

template<typename T>
class Slider
{
	sf::FloatRect m_bounding_box;
	sf::RectangleShape m_handle;
	sf::RectangleShape m_line;

	T* m_value;
	const T m_max;
	const T m_min;
	T m_prev_val;
	
	bool m_pressed{ false };

	void handle_new_value(const float percentage)
	{
		const float offset = m_bounding_box.size.x * percentage;
		const float handle_center_x = m_bounding_box.position.x + offset - m_handle.getSize().x / 2.f;

		m_handle.setPosition({ handle_center_x, m_bounding_box.position.y });

		const float line_center_y = m_bounding_box.position.y + m_bounding_box.size.y / 2.0f;
		m_line.setPosition({ m_bounding_box.position.x,line_center_y - 5.f / 2.f });
	}

public:

	Slider(T* value, T min, T max, const sf::Vector2f& position, const sf::Vector2f& size) :m_bounding_box{ position,size }, m_handle({ 20.f,size.y }), m_line({ size.x,5.f })
		, m_value(value), m_max(max), m_min(min)
	{
		const float percentage = static_cast<float>((*value - min)) / static_cast<float>((max - min));
		handle_new_value(percentage);

	}


	void update(const std::shared_ptr<Eventsystem>& eventsystem)
	{
		const sf::Vector2f mouse_pos = eventsystem->get_mouse_position();
		const bool is_mouse_on_slider = m_bounding_box.contains(mouse_pos);
		if (eventsystem->get_mouse_button_action(sf::Mouse::Button::Left) == Eventsystem::action_pressed && is_mouse_on_slider)
		{
			m_pressed = true;
		}
		else if (eventsystem->get_mouse_button_state(sf::Mouse::Button::Left) == false || !m_pressed)
		{
			m_pressed = false;
			if (m_prev_val != *m_value)
			{
				const float percentage = static_cast<float>((*m_value - m_min)) / static_cast<float>((m_max - m_min));
				handle_new_value(percentage);
			}
			m_prev_val = *m_value;
			return;
		}

		const sf::Vector2f mouse_pos_as_offset = mouse_pos - m_bounding_box.position;
		const float percentage = std::clamp(mouse_pos_as_offset.x / m_bounding_box.size.x, 0.f, 1.f);
		m_prev_val = percentage * static_cast<float>((m_max - m_min)) + m_min;
		*m_value = m_prev_val;
		handle_new_value(percentage);
	}

	void set_value(T* new_value)
	{
		m_value = new_value;
		if (m_prev_val != *m_value)
		{
			const float percentage = static_cast<float>((*m_value - m_min)) / static_cast<float>((m_max - m_min));
			handle_new_value(percentage);
			m_prev_val = *m_value;
		}
	}

	bool is_clicked() const { return m_pressed; }

	void set_position(const sf::Vector2f& position)
	{
		if (position == m_bounding_box.position)
			return;
		m_bounding_box.position = position;
		const float percentage = static_cast<float>((*m_value - m_min)) / static_cast<float>((m_max - m_min));
		handle_new_value(percentage);
	}
	sf::Vector2f get_position() const { return m_bounding_box.position; }

	void render(sf::RenderWindow& window) const
	{
		window.draw(m_line);
		window.draw(m_handle);
	}

	T* get_value() { return m_value; }
	T get_min() const { return m_min; }
	T get_max() const { return m_max; }

};
