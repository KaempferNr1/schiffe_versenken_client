#include "VersusCom.h"
#include "Eventsystem.h"
#include "imgui.h"
#include "LayerManager.h"
#include "UI/Button.h"
#include "PauseMenu.h"
#include "Utils/Log.h"

VersusCom::VersusCom()
{
	m_computer = std::make_unique<Computer>();
	if (!m_player_background_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "oceangrid_final.png"))LOG_ERROR("Failed to load");
	if (!m_opponent_background_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "radargrid_final.png"))LOG_ERROR("Failed to load");
	if (!m_tokens_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "Tokens 1.png"))LOG_ERROR("Failed to load");
	if (!m_ship_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "BattleShipSheet_final.png"))LOG_ERROR("Failed to load");

	m_tokens_vertex_array.setPrimitiveType(sf::PrimitiveType::Triangles);

	for (Ship& ship : m_computer->m_ships)
	{
		ship.sprite.setTexture(&m_ship_texture);
	}
}

void VersusCom::update(std::shared_ptr<Eventsystem>& eventsystem, std::shared_ptr<LayerManager>& layer_manager,
	std::shared_ptr<Soundsystem>& soundsystem, sf::RenderWindow& window, double deltatime)
{
	if (m_status == status::GAME_DONE)
	{
		if (eventsystem->get_key_action(sf::Keyboard::Key::Enter) == Eventsystem::action_pressed)
		{
			m_computer = std::make_unique<Computer>();
			for (Ship& ship : m_computer->m_ships)
			{
				ship.sprite.setTexture(&m_ship_texture);
			}
			ships_left = {1,2,1,1};
			m_ships.clear();
			m_shots = {0};
			m_ship_map = {0};
			m_tokens_vertex_array.clear();
			m_status = status::PLACING_PHASE;
			m_player_ships_remaining = 5;
			m_computer_ships_remaining = 5;
		}
	}
	else if (m_status == status::PLACING_PHASE)
	{
		ImGui::Begin("debug");

		if (eventsystem->get_key_action(sf::Keyboard::Key::Down) == Eventsystem::action_pressed || eventsystem->get_key_action(sf::Keyboard::Key::S) == Eventsystem::action_pressed)
			m_row = (m_row + 1) % 10;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Up) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::W) == Eventsystem::action_pressed)
			m_row = m_row - 1 < 0 ? 9 : m_row - 1;

		if (eventsystem->get_key_action(sf::Keyboard::Key::Right) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::D) == Eventsystem::action_pressed)
			m_col = (m_col + 1) % 10;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Left) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::A) == Eventsystem::action_pressed)
			m_col = m_col - 1 < 0 ? 9 : m_col - 1;

		if (eventsystem->get_key_action(sf::Keyboard::Key::R) == Eventsystem::action_pressed)
			m_is_horizontal = !m_is_horizontal;
		if (eventsystem->get_key_action(sf::Keyboard::Key::E) == Eventsystem::action_pressed)
			m_length = m_length + 1 > 5 ? 2 : m_length + 1;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Q) == Eventsystem::action_pressed)
			m_length = m_length - 1 < 2 ? 5 : m_length - 1;

		ImGui::SliderInt("row", &m_row, 0, 9);
		ImGui::SliderInt("col", &m_col, 0, 9);
		ImGui::SliderInt("length", &m_length, 2, 5);
		ImGui::Checkbox("is horizontal", &m_is_horizontal);
		if (ImGui::Button("place ship") || eventsystem->get_key_action(sf::Keyboard::Key::Enter) == Eventsystem::action_pressed)
		{
			if (validplacement(m_row,m_col,m_length,m_is_horizontal,m_ship_map) && !(m_length < 2 || m_length > 5) && ships_left[m_length - 2] > 0)
			{
				ships_left[m_length - 2]--;
				m_ships.emplace_back(place_ship(m_row, m_col, m_length, m_is_horizontal, m_ship_map, m_ships.size(), player_map_offset)).sprite.setTexture(&m_ship_texture);
			}
		}

		ImGui::End();
		if (m_ships.size() == 5)
		{
			m_status = status::SHOOTING_PHASE;
		}
	}
	else
	{
		ImGui::Begin("debug");
		if (eventsystem->get_key_action(sf::Keyboard::Key::Down) == Eventsystem::action_pressed || eventsystem->get_key_action(sf::Keyboard::Key::S) == Eventsystem::action_pressed)
			m_row = (m_row + 1) % 10;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Up) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::W) == Eventsystem::action_pressed)
			m_row = m_row - 1 < 0 ? 9 : m_row - 1;

		if (eventsystem->get_key_action(sf::Keyboard::Key::Right) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::D) == Eventsystem::action_pressed)
			m_col = (m_col + 1) % 10;
		if (eventsystem->get_key_action(sf::Keyboard::Key::Left) == Eventsystem::action_pressed
			|| eventsystem->get_key_action(sf::Keyboard::Key::A) == Eventsystem::action_pressed)
			m_col = m_col - 1 < 0 ? 9 : m_col - 1;

		ImGui::SliderInt("row", &m_row, 0, 9);
		ImGui::SliderInt("col", &m_col, 0, 9);

		if (ImGui::Button("shoot") || eventsystem->get_key_action(sf::Keyboard::Key::Enter) == Eventsystem::action_pressed)
		{
			while ((!(m_col < 0 || m_col > 9) && !(m_row < 0 || m_row > 9)))
			{
				if (m_shots[m_row][m_col] != 0)
				{
					break;
				}
				
				{
					const int8_t cell = m_computer->m_ship_map[m_row][m_col];
					int8_t hit_type = cell == 0 ? 1 : 2; // 1 == miss 2 == hit
					m_shots[m_row][m_col] = hit_type;
					if (hit_type == 2)
					{
						m_computer->m_ships[cell - 1].segments_left--;
						if (m_computer->m_ships[cell - 1].segments_left == 0) 
						{
							m_computer->m_ships[cell - 1].destroyed = true;
							m_computer_ships_remaining--;
						}

					}
					add_token_to_vertex_array(m_tokens_vertex_array, m_row, m_col, hit_type - 1, opponent_map_offset + cell_size);
					if (m_computer_ships_remaining == 0)
						break;
				}
				
				{
					auto [computer_row, computer_col] = m_computer->make_move();
					if (computer_row < 0 || computer_row > 9 || computer_col < 0 || computer_col > 9)
					{
						__debugbreak();
					}
					const int8_t cell = m_ship_map[computer_row][computer_col];
					int8_t hit_type = cell == 0 ? 1 : 2; // 1 == miss 2 == hit
					m_computer->m_shots[computer_row][computer_col] = hit_type;
					if (hit_type == 2)
					{
						m_ships[cell - 1].segments_left--;
						if (m_ships[cell - 1].segments_left == 0)
						{
							m_ships[cell - 1].destroyed = true;
							m_player_ships_remaining--;
							for (auto [row,col] : m_ships[cell-1].coordinates)
							{
								m_computer->m_shots[row][col] = 3;
							}
							m_computer->shipsleft[m_ships[cell - 1].coordinates.size()-2]--;
						}
					}
					add_token_to_vertex_array(m_tokens_vertex_array, computer_row, computer_col, hit_type + 1, player_map_offset + cell_size);

				}
				break;
			}
		}

		if (m_computer_ships_remaining == 0 || m_player_ships_remaining == 0)
		{
			m_status = status::GAME_DONE;

		}

		ImGui::End();
	}

	if (eventsystem->get_key_action(sf::Keyboard::Key::Escape) == Eventsystem::action_pressed)
	{
		layer_manager->push_layer(std::make_shared<PauseMenu>(layer_manager->get_top()));
		return;
	}
}

void VersusCom::render(sf::RenderWindow& window)
{
	sf::RectangleShape rect;
	rect.setPosition(player_map_offset);
	rect.setTexture(&m_player_background_texture);
	rect.setSize(map_size);
	window.draw(rect);

	if (m_status != status::PLACING_PHASE)
	{
		rect.setPosition(opponent_map_offset);
		rect.setTexture(&m_opponent_background_texture);
		window.draw(rect);
	}

	for (Ship& ship : m_ships)
	{
		ship.render(window);
	}
	if (m_status == status::PLACING_PHASE)
	{

		sf::RectangleShape selection = create_ship_sprite(m_row, m_col, m_length, m_is_horizontal, player_map_offset);
		selection.setTexture(&m_ship_texture);
		selection.setFillColor({ 255,255,255,127 });
		window.draw(selection);
		return;
	}
	sf::RenderStates states = sf::RenderStates::Default;
	states.texture = &m_tokens_texture;
	window.draw(m_tokens_vertex_array, states);

	if (m_status == status::SHOOTING_PHASE)
	{
		sf::RectangleShape selection{ {32.f,32.f} };
		selection.setPosition(calculate_position(m_row, m_col, opponent_map_offset));
		selection.setFillColor({ 127,127,127,127 });
		window.draw(selection);
	}
	else
	{
		for (Ship& ship : m_computer->m_ships)
		{
			ship.render(window);
		}
	}
}

void VersusCom::on_close()
{

}

LayerID VersusCom::get_layer_id()
{
	return LayerID::game;
}
