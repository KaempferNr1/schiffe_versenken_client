#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

#include "ButtonBehaviour.h"
#include "Eventsystem.h"
#include "UI/Slider.h"

namespace sf
{
	class RenderWindow;
}
class Soundsystem;
class ButtonLayout
{
public:
	ButtonLayout() = default;
	ButtonLayout& operator=(const ButtonLayout&) = default;
	ButtonLayout(const ButtonLayout&) = default;
	ButtonLayout& operator=(ButtonLayout&&) noexcept = default;
	ButtonLayout(ButtonLayout&&) noexcept = default;

	virtual ~ButtonLayout() = default;
	virtual void update(const std::shared_ptr<Eventsystem>& eventsystem) = 0;
	virtual void set_position(const sf::Vector2f& position) = 0;
	virtual sf::Vector2f get_position() = 0;
	[[nodiscard]] virtual bool is_hovered() = 0; 
	[[nodiscard]] virtual bool is_clicked() = 0;
	virtual void set_is_hovered(bool hovered) = 0;
	virtual void render(sf::RenderWindow& window) = 0;
	virtual void set_data(const std::string& i_string) = 0;
};

class EmptyLayout : public ButtonLayout
{
	sf::Vector2f m_position{ -1.f,-1.f };
public:
	void update([[maybe_unused]] const std::shared_ptr<Eventsystem>& eventsystem) override {}
	void set_position([[maybe_unused]] const sf::Vector2f& position) override { m_position = position; }
	sf::Vector2f get_position() override { return m_position; }
	[[nodiscard]] bool is_hovered()  override {return false;}
	[[nodiscard]] bool is_clicked()  override {return false;}
	void set_is_hovered([[maybe_unused]] bool hovered)  override {}
	void render([[maybe_unused]] sf::RenderWindow& window)  override {}
	void set_data([[maybe_unused]] const std::string& i_text) override {}
};

class TextLayout : public ButtonLayout
{
public:
	TextLayout(
		const std::string& i_text,
		const sf::Vector2f& i_position,
		const sf::Vector2f& i_size,
		float i_outline_thickness = 2.f,
		const sf::Color& i_default_color = sf::Color::White,
		const sf::Color& i_hovered_color = sf::Color(200, 200, 200),
		const sf::Color& i_pressed_color = sf::Color(150,150,150),
		const sf::Color& i_outline_color = sf::Color::Black
	);
	void update(const std::shared_ptr<Eventsystem>& eventsystem) override;
	void set_position(const sf::Vector2f& position) override;
	sf::Vector2f get_position() override;
	[[nodiscard]] bool is_hovered() override;
	[[nodiscard]] bool is_clicked() override;
	void set_is_hovered(bool hovered) override;
	void render(sf::RenderWindow& window) override;
	void set_data(const std::string& i_text) override;
private:
	sf::Font m_font;
	sf::Text m_text;
	sf::RectangleShape m_shape;
	bool m_hovered{ false };
	bool m_clicked{ false };
	sf::Color m_default_color;
	sf::Color m_hovered_color;
	sf::Color m_pressed_color;
};

class TextImageLayout : public ButtonLayout
{
public:
	TextImageLayout(
		const std::string& i_text,
		const sf::Vector2f& i_position,
		const sf::Vector2f& i_size,
		sf::Texture* i_default_texture,
		sf::Texture* i_hovered_texture,
		sf::Texture* i_pressed_texture,
		float i_outline_thickness = 2.f,
		const sf::Color& i_outline_color = sf::Color::Black
	);
	void update(const std::shared_ptr<Eventsystem>& eventsystem) override;
	void set_position(const sf::Vector2f& position) override;
	sf::Vector2f get_position() override;
	[[nodiscard]] bool is_hovered() override;
	[[nodiscard]] bool is_clicked() override;
	void set_is_hovered(bool hovered) override;
	void render(sf::RenderWindow& window) override;
	void set_data(const std::string& i_text) override;
private:
	sf::Font m_font;
	sf::Text m_text;
	sf::RectangleShape m_shape;
	bool m_hovered{ false };
	bool m_clicked{ false };
	sf::Texture* m_default_texture;
	sf::Texture* m_hovered_texture;
	sf::Texture* m_pressed_texture;
};

class FloatSliderLayout : public ButtonLayout
{
public:
	FloatSliderLayout(const std::shared_ptr<Slider<float>>& i_slider);
	void update(const std::shared_ptr<Eventsystem>& eventsystem) override;
	void set_position(const sf::Vector2f& position) override;
	sf::Vector2f get_position() override;
	[[nodiscard]] bool is_hovered() override;
	[[nodiscard]] bool is_clicked() override;
	void set_is_hovered(bool hovered) override;
	void render(sf::RenderWindow& window) override;
	void set_data(const std::string& i_text) override;
private:
	std::shared_ptr<Slider<float>> m_slider;
	bool m_hovered{ false };
	bool m_clicked{ false };
};