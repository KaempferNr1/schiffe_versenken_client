#include "VersusCom.h"
#include "Eventsystem.h"
#include "imgui.h"
#include "LayerManager.h"
#include "UI/Button.h"
#include "PauseMenu.h"
#include "Utils/Log.h"

constexpr double hit_weight{ 1.73226 };
constexpr double unused_weight{ 8.08751 };
constexpr double smart_change{ 8 };

void Computer::place_ships()
{
	int row;
	int col;
	const std::array<int, 5> wahl = { 2,3,3,4,5 };

	std::string waiter;
	for (int i = 0; i < 5; i++)
	{
		const bool is_horizontal = Random::foo(0.5);
		const int length = wahl[i];
		do
		{
			row = Random::uint(0, 9);
			col = Random::uint(0, 9);
		} while (!validplacement(row, col, wahl[i], is_horizontal, m_ship_map));
		m_ships[i] = place_ship(row, col, length, is_horizontal, m_ship_map, i, opponent_map_offset);
	}
}

Computer::Computer()
{
	place_ships();
}

void Computer::change_probs(int col, int row, double change, bool do_smarter_check)
{

	m_probabilities[row][col] = 0;
	if (row != 9)
	{
		if (!m_shots[static_cast<long long>(row) + 1][static_cast<long long>(col)])
		{
			m_probabilities[static_cast<long long>(row) + 1][static_cast<long long>(col)] += change;
		}
		if (m_shots[static_cast<long long>(row) + 1][static_cast<long long>(col)] == 2)
		{
			if (row != 0 && !m_shots[static_cast<long long>(row) - 1][static_cast<long long>(col)])
			{
				m_probabilities[static_cast<long long>(row) - 1][static_cast<long long>(col)] += smart_change;
			}
		}
	}
	if (col != 9)
	{
		if (!m_shots[static_cast<long long>(row)][static_cast<long long>(col) + 1])
		{
			m_probabilities[static_cast<long long>(row)][static_cast<long long>(col) + 1] += change;
		}
		if (m_shots[static_cast<long long>(row)][static_cast<long long>(col) + 1] == 2 && do_smarter_check)
		{
			if (col != 0 && !m_shots[static_cast<long long>(row)][static_cast<long long>(col) - 1])
			{
				m_probabilities[static_cast<long long>(row)][static_cast<long long>(col) - 1] += smart_change;
			}
		}
	}
	if (row != 0)
	{
		if (!m_shots[static_cast<long long>(row) - 1][static_cast<long long>(col)])
		{
			m_probabilities[static_cast<long long>(row) - 1][static_cast<long long>(col)] += change;
		}
		if (m_shots[static_cast<long long>(row) - 1][static_cast<long long>(col)] == 2 && do_smarter_check)
		{
			if (row != 9 && !m_shots[static_cast<long long>(row) + 1][static_cast<long long>(col)])
			{
				m_probabilities[static_cast<long long>(row) + 1][static_cast<long long>(col)] += smart_change;
			}
		}

	}
	if (col != 0)
	{
		if (!m_shots[static_cast<long long>(row)][static_cast<long long>(col) - 1])
		{
			m_probabilities[static_cast<long long>(row)][static_cast<long long>(col) - 1] += change;
		}
		if (m_shots[static_cast<long long>(row)][static_cast<long long>(col) - 1] == 2)
		{
			if (col != 9 && !m_shots[static_cast<long long>(row)][static_cast<long long>(col) + 1])
			{
				m_probabilities[static_cast<long long>(row)][static_cast<long long>(col) + 1] += smart_change;
			}
		}
	}

}

std::pair<int, int> Computer::make_move()
{
	int next_row = 0;
	int next_col = 0;
	for (int row = 0; row < 10; row++)
	{
		for (int col = 0; col < 10; col++)
		{
			if (!m_shots[row][col] && m_probabilities[row][col] != 0.0)
			{
				m_probabilities[row][col] = 1.0;
			}
			else
			{
				m_probabilities[row][col] = 0.0;
			}
		}
	}

	
	bool foundhit = false;
	for (int row = 0; row < 10; row++)
	{
		for (int col = 0; col < 10; col++)
		{
			const int8_t cell = m_shots[row][col];
			if (cell == 2)
			{
				change_probs(col, row, hit_weight, true);
				foundhit = true;
			}
		}
	}
	if (!foundhit)
	{
		std::unordered_map<int, int> ships;
		for (int i = 0; i < 4; i++)
		{
			ships[i + 2] = shipsleft[i];
		}
		for (int row = 0; row < 10; row++)
		{
			for (int col = 0; col < 10; col++)
			{
				if (m_shots[row][col] != 0)
				{
					continue;
				}
				for (const auto& [shiplen, amount_ships_left] : ships)
				{
					if (amount_ships_left <= 0)
					{
						continue;
					}

					if (validplacement(row, col, shiplen, true, m_shots))
					{
						for (int j = 0; j < shiplen; j++)
						{
							m_probabilities[row][col + j] += (shiplen + j) * unused_weight;
						}
					}
					else
					{
						for (int j = 0; j < shiplen; j++)
						{
							if (col + j < 0 || col + j > 9)
							{
								break;
							}
							m_probabilities[row][col + j] -= ((shiplen - j) * unused_weight) / hit_weight;
						}
					}

					if (validplacement(row, col, shiplen, false, m_shots))
					{
						for (int j = 0; j < shiplen; j++)
						{
							m_probabilities[row + j][col] += (shiplen + j) * unused_weight;
						}
					}
					else
					{
						for (int j = 0; j < shiplen; j++)
						{
							if (row + j < 0 || row + j > 9)
							{
								break;
							}
							m_probabilities[row + j][col] -= ((shiplen - j) * unused_weight) / hit_weight;
						}
					}
				}
			}
		}
	}

	double max_prob = std::numeric_limits<double>::lowest();
	std::pair<int, int> highest_candidate;
	bool tie = false;
	std::vector<std::pair<int, int>> tie_candidates;
	tie_candidates.reserve(10);
	for (int row = 0; row < 10; row++)
	{
		for (int col = 0; col < 10; col++)
		{
			if (m_shots[row][col] != 0)
			{
				continue;
			}
			const double probability = m_probabilities[row][col];
			const double difference = abs(probability - max_prob);
			if (m_probabilities[row][col] > max_prob)
			{
				max_prob = m_probabilities[row][col];
				tie = false;
				tie_candidates.clear();
				highest_candidate = std::make_pair(row, col);
			}
			else if (difference < 1e-010)
			{
				tie = true;
				tie_candidates.emplace_back(row, col);
			}
		}
	}

	if (tie) {
		const std::pair<int, int> res = tie_candidates[Random::uint(0, static_cast<uint32_t>(tie_candidates.size()) - 1)];
		next_row = res.first;
		next_col = res.second;
	}
	else {
		next_row = highest_candidate.first;
		next_col = highest_candidate.second;
	}
	return { next_row, next_col };
}

VersusCom::VersusCom()
{
	m_computer = std::make_unique<Computer>();
	if (!m_player_background_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "oceangrid_final.png"))LOG_ERROR("Failed to load");
	if (!m_opponent_background_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "radargrid_final.png"))LOG_ERROR("Failed to load");
	if (!m_tokens_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "Tokens 2.png"))LOG_ERROR("Failed to load");
	if (!m_ship_texture.loadFromFile(std::filesystem::path("Resources") / "Images" / "Naval Battle Assets" / "BattleShipSheet_final.png"))LOG_ERROR("Failed to load");

	m_tokens_vertex_array.setPrimitiveType(sf::PrimitiveType::Triangles);

	for (Ship& ship : m_computer->m_ships)
	{
		ship.sprite.setTexture(&m_ship_texture);
	}
}

void VersusCom::update(std::shared_ptr<Eventsystem>& eventsystem, std::shared_ptr<LayerManager>& layer_manager,
	[[maybe_unused]]std::shared_ptr<Soundsystem>& soundsystem, [[maybe_unused]] sf::RenderWindow& window, [[maybe_unused]] double deltatime)
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
		if (place_ship_screen(eventsystem,m_row,m_col,m_is_horizontal,m_length))
		{
			if (validplacement(m_row,m_col,m_length,m_is_horizontal,m_ship_map) && !(m_length < 2 || m_length > 5) && ships_left[m_length - 2] > 0)
			{
				ships_left[m_length - 2]--;
				m_ships.emplace_back(place_ship(m_row, m_col, m_length, m_is_horizontal, m_ship_map, static_cast<int>(m_ships.size()), player_map_offset)).sprite.setTexture(&m_ship_texture);
			}
		}

		if (m_ships.size() == 5)
		{
			m_status = status::SHOOTING_PHASE;
		}
	}
	else
	{
		if (shoot_screen(eventsystem,m_row,m_col))
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
							hit_type = 5;
							for (auto [row, col] : m_computer->m_ships[cell - 1].coordinates)
							{
								m_shots[row][col] = 3;
								change_token_from_vertex_array(m_tokens_vertex_array, row, col, hit_type - 1, opponent_map_offset + cell_size);
							}
						}

					}
					add_token_to_vertex_array(m_tokens_vertex_array, m_row, m_col, hit_type - 1, opponent_map_offset + cell_size);
					if (m_computer_ships_remaining == 0)
						break;
				}
				
				{
					auto [computer_row, computer_col] = m_computer->make_move();
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

	if (m_status == status::GAME_DONE)
	{
		for (Ship& ship : m_computer->m_ships)
		{
			ship.render(window);
		}
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

}

void VersusCom::on_close()
{

}

LayerID VersusCom::get_layer_id()
{
	return LayerID::versus_com;
}
