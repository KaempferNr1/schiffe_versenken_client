#include "OptionsMenu.h"

#include "UI/Button.h"
#include "Eventsystem.h"
#include "LayerManager.h"
#include "Utils/Log.h"
#include "Utils/Soundsystem.h"
#include "imgui.h"
#include <format>

OptionsMenu::OptionsMenu(const std::shared_ptr<Soundsystem>& soundsystem)
{
	for (int i = 0; i < 20; i++)
	{
		m_buttons.emplace_back(std::make_unique<Button>());
	}

	m_buttons[0]->set_layout(std::make_shared<TextLayout>("back", sf::Vector2f{ 260.f,145.f }, Button::medium_button_size, 0.f));
	m_buttons[0]->set_behaviour(std::make_shared<PopLayer>());

	m_buttons[1]->set_layout(std::make_shared<TextLayout>("<", sf::Vector2f{ 210.f,215.f }, Button::small_button_size, 0.f));
	m_buttons[1]->set_behaviour(std::make_shared<EmptyBehaviour>());

	m_buttons[2]->set_layout(std::make_shared<TextLayout>("WindowMode", sf::Vector2f{ 260.f,215.f }, Button::medium_button_size, 0.f));
	m_buttons[2]->set_behaviour(std::make_shared<EmptyBehaviour>());

	m_buttons[3]->set_layout(std::make_shared<TextLayout>(">", sf::Vector2f{ 460.f,215.f }, Button::small_button_size, 0.f));
	m_buttons[3]->set_behaviour(std::make_shared<EmptyBehaviour>());

	//GLOBAL volume
	m_buttons[4]->set_layout(std::make_shared<TextLayout>("global: 100", sf::Vector2f{ 260.f,285.f }, Button::medium_button_size, 0.f));
	m_buttons[4]->set_behaviour(std::make_shared<EmptyBehaviour>());

	m_buttons[5]->set_layout(std::make_shared<TextLayout>("<", sf::Vector2f{ 210.f,285.f }, Button::small_button_size, 0.f));
	m_buttons[5]->set_behaviour(std::make_shared<IncrementVolume>("global", -5.f));

	m_buttons[6]->set_layout(std::make_shared<TextLayout>(">", sf::Vector2f{ 460.f,285.f }, Button::small_button_size, 0.f));
	m_buttons[6]->set_behaviour(std::make_shared<IncrementVolume>("global", 5.f));

	std::shared_ptr<Slider<float>> global_slider = std::make_shared<Slider<float>>(&soundsystem->get_volumes().at("global"), 0.f, 100.f, sf::Vector2f{ 260.f,285.f }, Button::medium_button_size);
	m_buttons[7]->set_layout(std::make_shared<FloatSliderLayout>(global_slider));
	m_buttons[7]->set_behaviour(std::make_shared<EmptyBehaviour>());

	//MUSIC volume

	m_buttons[8]->set_layout(std::make_shared<TextLayout>("music: 100", sf::Vector2f{ 260.f,285.f + 50.f }, Button::medium_button_size, 0.f));
	m_buttons[8]->set_behaviour(std::make_shared<EmptyBehaviour>());

	m_buttons[9]->set_layout(std::make_shared<TextLayout>("<", sf::Vector2f{ 210.f,285.f + 50.f }, Button::small_button_size, 0.f));
	m_buttons[9]->set_behaviour(std::make_shared<IncrementVolume>("music", -5.f));
	
	m_buttons[10]->set_layout(std::make_shared<TextLayout>(">", sf::Vector2f{ 460.f,285.f + 50.f }, Button::small_button_size, 0.f));
	m_buttons[10]->set_behaviour(std::make_shared<IncrementVolume>("music", 5.f));

	std::shared_ptr<Slider<float>> music_slider = std::make_shared<Slider<float>>(&soundsystem->get_volumes().at("music"), 0.f, 100.f, sf::Vector2f{ 260.f,285.f }, Button::medium_button_size);
	m_buttons[11]->set_layout(std::make_shared<FloatSliderLayout>(music_slider));
	m_buttons[11]->set_behaviour(std::make_shared<EmptyBehaviour>());

	//UI SOUNDS

	m_buttons[12]->set_layout(std::make_shared<TextLayout>("ui: 100", sf::Vector2f{ 260.f,285.f + 100.f }, Button::medium_button_size, 0.f));
	m_buttons[12]->set_behaviour(std::make_shared<EmptyBehaviour>());

	m_buttons[13]->set_layout(std::make_shared<TextLayout>("<", sf::Vector2f{ 210.f,285.f + 100.f }, Button::small_button_size, 0.f));
	m_buttons[13]->set_behaviour(std::make_shared<IncrementVolume>("ui_sounds", -5.f));

	m_buttons[14]->set_layout(std::make_shared<TextLayout>(">", sf::Vector2f{ 460.f,285.f + 100.f }, Button::small_button_size, 0.f));
	m_buttons[14]->set_behaviour(std::make_shared<IncrementVolume>("ui_sounds", 5.f));

	std::shared_ptr<Slider<float>> ui_slider = std::make_shared<Slider<float>>(&soundsystem->get_volumes().at("ui_sounds"), 0.f, 100.f, sf::Vector2f{ 260.f,385.f }, Button::medium_button_size);
	m_buttons[15]->set_layout(std::make_shared<FloatSliderLayout>(ui_slider));
	m_buttons[15]->set_behaviour(std::make_shared<EmptyBehaviour>());


	//GAME SOUNDS
	m_buttons[16]->set_layout(std::make_shared<TextLayout>("player: 100", sf::Vector2f{ 260.f,285.f + 150.f }, Button::medium_button_size, 0.f));
	m_buttons[16]->set_behaviour(std::make_shared<EmptyBehaviour>());
	
	m_buttons[17]->set_layout(std::make_shared<TextLayout>("<", sf::Vector2f{ 210.f,285.f + 150.f }, Button::small_button_size, 0.f));
	m_buttons[17]->set_behaviour(std::make_shared<IncrementVolume>("game_sounds", -5.f));
	
	m_buttons[18]->set_layout(std::make_shared<TextLayout>(">", sf::Vector2f{ 460.f,285.f + 150.f }, Button::small_button_size, 0.f));
	m_buttons[18]->set_behaviour(std::make_shared<IncrementVolume>("game_sounds", 5.f));

	std::shared_ptr<Slider<float>> game_slider = std::make_shared<Slider<float>>(&soundsystem->get_volumes().at("game_sounds"), 0.f, 100.f, sf::Vector2f{ 260.f,435.f }, Button::medium_button_size);
	m_buttons[19]->set_layout(std::make_shared<FloatSliderLayout>(game_slider));
	m_buttons[19]->set_behaviour(std::make_shared<EmptyBehaviour>());
	

	LOG_INFO("created options menu");

	//m_sliders.push_back(std::make_unique<Slider<float>>(&soundsystem->get_volumes().at("global"), 0.f, 100.f, sf::Vector2f{ 260.f,285.f }, sf::Vector2f{ 200.f,50.f }));
	//m_sliders.push_back(std::make_unique<Slider<float>>(&soundsystem->get_volumes().at("music"), 0.f, 100.f, sf::Vector2f{ 260.f,335.f }, sf::Vector2f{ 200.f,50.f }));
	//m_sliders.push_back(std::make_unique<Slider<float>>(&soundsystem->get_volumes().at("ui_sounds"), 0.f, 100.f, sf::Vector2f{ 260.f,385.f }, sf::Vector2f{ 200.f,50.f }));
	//m_sliders.push_back(std::make_unique<Slider<float>>(&soundsystem->get_volumes().at("game_sounds"), 0.f, 100.f, sf::Vector2f{ 260.f,435.f }, sf::Vector2f{ 200.f,50.f }));
	

}

void OptionsMenu::update(std::shared_ptr<Eventsystem>& eventsystem, std::shared_ptr<LayerManager>& layer_manager,
	std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window, [[maybe_unused]] double deltatime)
{
	if (eventsystem->get_key_action(sf::Keyboard::Key::Escape) == Eventsystem::action_pressed)
	{
		layer_manager->pop_layer();
		return;
	}

	constexpr float padding_y = 20.0f;
	constexpr float button_size_y = Button::medium_button_size.y;
	constexpr float buttons_in_y = 6.f;
	constexpr float total_height = buttons_in_y * button_size_y + (buttons_in_y - 1.f) * padding_y;
	const float start_y = (static_cast<float>(eventsystem->get_window_size().y) - total_height) / 2.f;

	constexpr float padding_x = 15.f;
	constexpr float total_width_sounds = Button::small_button_size.x * 2 + Button::medium_button_size.x * 2 + 3 * padding_x;
	constexpr std::array button_offsets_x_sounds{0.f,Button::medium_button_size.x + padding_x ,
		Button::medium_button_size.x * 2 + Button::small_button_size.x + padding_x * 3,
		Button::medium_button_size.x + Button::small_button_size.x + padding_x * 2
	};

	const float start_x_sounds = (static_cast<float>(eventsystem->get_window_size().x) - total_width_sounds) / 2.f;

	constexpr float total_width = Button::small_button_size.x * 2 + Button::medium_button_size.x + 2 * padding_x;
	constexpr std::array button_offsets_x{ 0.f,Button::small_button_size.x + padding_x,
		Button::small_button_size.x + Button::medium_button_size.x + padding_x * 2};
	const float start_x = (static_cast<float>(eventsystem->get_window_size().x) - total_width) / 2.f;

	m_buttons[4]->set_data(std::format("global: {:03.0f}", soundsystem->get_volumes().at("global")));
	m_buttons[8]->set_data(std::format("music: {:03.0f}", soundsystem->get_volumes().at("music")));
	m_buttons[12]->set_data(std::format("ui: {:03.0f}", soundsystem->get_volumes().at("ui_sounds")));
	m_buttons[16]->set_data(std::format("player: {:03.0f}", soundsystem->get_volumes().at("game_sounds")));
	for (uint8_t i = 0; i < m_buttons.size(); i++)
	{

		if (i == 0)
			m_buttons[i]->set_position(sf::Vector2f{ static_cast<float>(eventsystem->get_window_size().x) / 2.f - Button::medium_button_size.x / 2.f,start_y });
		else if (i < 4)
			m_buttons[i]->set_position(sf::Vector2f{ start_x + (button_offsets_x[(i + 2) % 3]),
				start_y + std::floor((static_cast<float>(i) + 2.f) / 3.f) * (button_size_y + padding_y) });
		else 
		{
			//m_buttons[i]->set_position(sf::Vector2f{ start_x_sounds,
			//	start_y + (i-2) * (button_size_y + padding_y) });
			m_buttons[i]->set_position(sf::Vector2f{ start_x_sounds + (button_offsets_x_sounds[i % 4]),
				start_y + std::floor((static_cast<float>(i) + 4.f) / 4.f) * (button_size_y + padding_y) });
		}

		m_buttons[i]->update(eventsystem);
		if (!m_buttons[i]->is_clicked())
			continue;
		if(i == 7 || i == 11 || i == 15 || i == 19)
		{
			soundsystem->trigger_volume_update();
		}

		if (m_buttons[i]->on_click(layer_manager, soundsystem, window))
		{
			m_selected = -1;
			return;
		}
	}

	if (m_buttons[1]->is_clicked() == true)
	{
		if (m_windowselect < 1)
			m_windowselect++;
		else
			m_windowselect = 0;
	}
	else if (m_buttons[3]->is_clicked() == true)
	{
		if (m_windowselect > 0)
			m_windowselect--;
		else
			m_windowselect = 1;
	}


	if (m_buttons[1]->is_clicked() || m_buttons[3]->is_clicked())
	{
		if (m_windowselect == 0)
		{
			m_buttons[2]->set_data("WindowMode");
			window.create(sf::VideoMode({ 720, 480 }), "window", sf::Style::Default);
		}
		else
		{
			m_buttons[2]->set_data("Fullscreen");
			window.create(sf::VideoMode({1920, 1080}), "window", sf::State::Fullscreen);
		}
		window.setFramerateLimit(60);
		LOG_INFO("windowselect: {}", m_windowselect);
	}
	

	if (eventsystem->get_key_action(sf::Keyboard::Key::Down) == Eventsystem::action_pressed)
		m_selected_y = m_selected_y + 1 == static_cast<int>(buttons_in_y) ? 0 : m_selected_y + 1;
	if (eventsystem->get_key_action(sf::Keyboard::Key::Up) == Eventsystem::action_pressed)
		m_selected_y = m_selected_y - 1 < 0 ? static_cast<int>(buttons_in_y)-1 : m_selected_y - 1;

	if (eventsystem->get_key_action(sf::Keyboard::Key::Right) == Eventsystem::action_pressed)
		m_selected_x = m_selected_x + 1 == 3 ? 0 : m_selected_x + 1;
	if (eventsystem->get_key_action(sf::Keyboard::Key::Left) == Eventsystem::action_pressed)
		m_selected_x = m_selected_x - 1 < 0 ? 2 : m_selected_x - 1;

	if ((m_selected_x == -1) ^ (m_selected_y == -1))
	{
		m_selected_x = m_selected_x == -1 ? 1 : m_selected_x;
		m_selected_y = m_selected_y == -1 ? 0 : m_selected_y;
	}

	if (eventsystem->get_mouse_button_action(sf::Mouse::Button::Left) == Eventsystem::action_pressed)
	{
		m_selected_x = -1;
		m_selected_y = -1;
	}

	if (m_selected_x == -1 || m_selected_y == -1)
	{
		return;
	}

	const int selected = m_selected_y == 0 ? 0 : m_selected_y == 1 ? m_selected_x + 1 + (m_selected_y-1) * 3 : m_selected_x + (m_selected_y - 1) * 4;

	m_buttons[selected]->set_is_hovered(true);

	if (eventsystem->get_key_action(sf::Keyboard::Key::Enter) == Eventsystem::action_released)
	{
		if (selected == 1 || selected == 3) 
		{
			if (selected == 1)
			{
				if (m_windowselect < 1)
					m_windowselect++;
				else
					m_windowselect = 0;
			}
			else
			{
				if (m_windowselect > 0)
					m_windowselect--;
				else
					m_windowselect = 1;
			}

			if (m_windowselect == 0)
			{
				m_buttons[2]->set_data("WindowMode");
				window.create(sf::VideoMode({720, 480}), "window", sf::Style::Default);
			}
			else
			{
				m_buttons[2]->set_data("Fullscreen");
				window.create(sf::VideoMode({1920, 1080}), "window", sf::State::Fullscreen);
			}
			window.setFramerateLimit(60);
			LOG_INFO("windowselect: {}", m_windowselect);
			return;
		}

		if (m_buttons[selected]->on_click(layer_manager, soundsystem, window))
		{
			m_selected_x = -1;
			m_selected_y = -1;
			return;
		}
	}

}

void OptionsMenu::render(sf::RenderWindow& window)
{

	for (const auto& button : m_buttons)
	{
		button->render(window);
	}
	//for (const auto& slider : m_sliders) 
	//{
	//	slider->render(window);
	//}
}

void OptionsMenu::on_close()
{
	LOG_INFO("closed OptionsMenu");
}

LayerID OptionsMenu::get_layer_id()
{
	return LayerID::options;
}
