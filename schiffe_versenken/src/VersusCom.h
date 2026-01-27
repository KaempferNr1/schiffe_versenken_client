#pragma once
#include <array>
#include <future>

#include "Game.h"
#include "PlacingScreen.h"
#include "Layer.h"
#include "Utils/Random.h"

constexpr double hit_weight{ 1.73226 };
constexpr double unused_weight{ 8.08751 };
constexpr double smart_change{ 8 };

class Computer
{
public:
	std::array<std::array<int8_t, 10>, 10> m_shots = { {0} };
	std::array<std::array<int8_t, 10>, 10> m_ship_map = { {0} };
	std::array<Ship, 5> m_ships;
	std::array<int, 4> shipsleft{ 1,2,1,1 };
private:
	std::vector<std::vector<double>> m_probabilities = std::vector<std::vector<double>>(10, std::vector<double>(10, 1));

	void place_ships()
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
public:
	Computer()
	{
		place_ships();
	}

	void change_probs(int col, int row, double change, bool do_smarter_check)
	{

		m_probabilities[row][col] = 0;
		if (row != 9)
		{
			if (!m_shots[(long long)row + 1][(long long)col])
			{
				m_probabilities[(long long)row + 1][(long long)col] += change;
			}
			if (m_shots[(long long)row + 1][(long long)col] == 2)
			{
				if (row != 0 && !m_shots[(long long)row - 1][(long long)col])
				{
					m_probabilities[(long long)row - 1][(long long)col] += smart_change;
				}
			}
		}
		if (col != 9)
		{
			if (!m_shots[(long long)row][(long long)col + 1])
			{
				m_probabilities[(long long)row][(long long)col + 1] += change;
			}
			if (m_shots[(long long)row][(long long)col + 1] == 2 && do_smarter_check)
			{
				if (col != 0 && !m_shots[(long long)row][(long long)col - 1])
				{
					m_probabilities[(long long)row][(long long)col - 1] += smart_change;
				}
			}
		}
		if (row != 0)
		{
			if (!m_shots[(long long)row - 1][(long long)col])
			{
				m_probabilities[(long long)row - 1][(long long)col] += change;
			}
			if (m_shots[(long long)row - 1][(long long)col] == 2 && do_smarter_check)
			{
				if (row != 9 && !m_shots[(long long)row + 1][(long long)col])
				{
					m_probabilities[(long long)row + 1][(long long)col] += smart_change;
				}
			}

		}
		if (col != 0)
		{
			if (!m_shots[(long long)row][(long long)col - 1])
			{
				m_probabilities[(long long)row][(long long)col - 1] += change;
			}
			if (m_shots[(long long)row][(long long)col - 1] == 2)
			{
				if (col != 9 && !m_shots[(long long)row][(long long)col + 1])
				{
					m_probabilities[(long long)row][(long long)col + 1] += smart_change;
				}
			}
		}

	}

	std::pair<int,int> make_move()
	{
		int next_row = 0;
		int next_col = 0;
		for (int row = 0; row < 10; row++)
		{
			for (int col = 0; col < 10; col++)
			{
				if (!m_shots[row][col] && m_probabilities[row][col] != 0.0)
				{
					m_probabilities[row][col] = 1;
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
					if (m_shots[row][col])
					{
						continue;
					}
					for (auto& [shiplen, shipleft] : ships)
					{
						if (!shipleft)
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
				if (!m_shots[row][col])
				{
					if (m_probabilities[row][col] > max_prob)
					{
						max_prob = m_probabilities[row][col];
						tie = false;
						tie_candidates.clear();
						highest_candidate = std::make_pair(row, col);
					}
					else if (m_probabilities[row][col] == max_prob)
					{
						tie = true;
						tie_candidates.emplace_back(row, col);
					}
				}
			}
		}

		if (tie) {
			std::uniform_int_distribution distro(0, (int)tie_candidates.size() - 1);
			const std::pair<int, int> res = tie_candidates[Random::uint(0, tie_candidates.size() - 1)];
			next_row = res.first;
			next_col = res.second;
		}
		else {
			next_row = highest_candidate.first;
			next_col = highest_candidate.second;
		}
		return { next_row, next_col };
	}
};

class VersusCom : public Layer
{
	std::unique_ptr<Computer> m_computer;
	std::vector<Ship> m_ships;

	sf::Texture m_player_background_texture;
	sf::Texture m_opponent_background_texture;
	sf::Texture m_tokens_texture;
	sf::Texture m_ship_texture;

	sf::VertexArray m_tokens_vertex_array;

	status m_status = status::PLACING_PHASE;
	std::array<std::array<int8_t, 10>, 10> m_shots = { {0} };
	std::array<std::array<int8_t, 10>, 10> m_ship_map = { {0} };
	std::array<int, 4> ships_left{1,2,1,1};
	int m_row{ 0 };
	int m_col{ 0 };
	int m_length{ 2 };
	bool m_is_horizontal{ false };

	int m_player_ships_remaining = 5;
	int m_computer_ships_remaining = 5;
public:
	VersusCom();
	void update([[maybe_unused]] std::shared_ptr<Eventsystem>& eventsystem, [[maybe_unused]] std::shared_ptr<LayerManager>& layer_manager,
		[[maybe_unused]] std::shared_ptr<Soundsystem>& soundsystem, [[maybe_unused]] sf::RenderWindow& window, [[maybe_unused]] double deltatime) override;

	void render([[maybe_unused]] sf::RenderWindow& window) override;

	void on_close() override;
	[[nodiscard]] LayerID get_layer_id() override;
};