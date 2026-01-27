#pragma once
#include <array>
#include <future>

#include "Client.h"
#include "Utils/Random.h"


inline bool validplacement(int row, int col, int length, bool is_horizontal, const std::array<std::array<int8_t, 10>, 10>& map)
{
	int amount_of_rows;
	int amount_of_cols;
	if (is_horizontal)
	{
		amount_of_cols = length;
		amount_of_rows = 1;
	}
	else
	{
		amount_of_cols = 1;
		amount_of_rows = length;
	}

	for (int offset_in_row = 0; offset_in_row < amount_of_rows; ++offset_in_row)
	{
		for (int offset_in_col = 0; offset_in_col < amount_of_cols; ++offset_in_col)
		{
			const int current_row = row + offset_in_row;
			const int current_col = col + offset_in_col;

			if (current_row < 0 || current_row > 9 ||
				current_col < 0 || current_col > 9)
			{
				return false;
			}

			if (map[current_row][current_col] != 0)
			{
				return false;
			}
		}
	}
	return true;
}

//class VersusCom
//{
//	std::array<std::array<int8_t, 10>, 10> m_shots = { {0} };
//	std::array<std::array<int8_t, 10>, 10> m_ship_map = { {0} };
//	std::vector<std::vector<double>> m_probabilities = std::vector<std::vector<double>>(10, std::vector<double>(10, 1));
//
//	std::array<Ship, 5> m_ships;
//	void place_ships()
//	{
//		int wahlR = 0;
//		int wahlS = 0;
//		std::array<int, 5> wahl = { 2,3,3,4,5 };
//		
//		std::string waiter;
//		std::bernoulli_distribution distr1(0.5);
//		std::uniform_int_distribution distr2(0, 9);
//		for (int i = 0; i < 5; i++)
//		{
//			bool is_horizontal = distr1(prob->randomengine);
//			do
//			{
//				wahlR = distr2(prob->randomengine);
//				wahlS = distr2(prob->randomengine);
//			} while (!validplacement(wahlR, wahlS, wahl[i], is_horizontal, m_ship_map));
//			place_ship()
//			ships ship = { wahlR, wahlS, wahl[i], is_horizontal };
//			shipsplacement.push_back(ship);
//		}
//		trefferuebrig = 17;
//	}
//public:
//	VersusCom()
//	{
//		place_ships();
//
//	}
//
//	void change_probs(int col, int row, double change, bool do_smarter_check)
//	{
//
//		m_probabilities[row][col] = 0;
//		if (row != 9)
//		{
//			if (!m_shots[(long long)row + 1][(long long)col])
//			{
//				m_probabilities[(long long)row + 1][(long long)col] += change;
//			}
//			if (m_shots[(long long)row + 1][(long long)col] == 2)
//			{
//				if (row != 0 && !m_shots[(long long)row - 1][(long long)col])
//				{
//					m_probabilities[(long long)row - 1][(long long)col] += smart_change;
//				}
//			}
//		}
//		if (col != 9)
//		{
//			if (!m_shots[(long long)row][(long long)col + 1])
//			{
//				m_probabilities[(long long)row][(long long)col + 1] += change;
//			}
//			if (m_shots[(long long)row][(long long)col + 1] == 2 && do_smarter_check)
//			{
//				if (col != 0 && !m_shots[(long long)row][(long long)col - 1])
//				{
//					m_probabilities[(long long)row][(long long)col - 1] += smart_change;
//				}
//			}
//		}
//		if (row != 0)
//		{
//			if (!m_shots[(long long)row - 1][(long long)col])
//			{
//				m_probabilities[(long long)row - 1][(long long)col] += change;
//			}
//			if (m_shots [(long long)row - 1][(long long)col] == 2 && do_smarter_check)
//			{
//				if (row != 9 && !m_shots[(long long)row + 1][(long long)col])
//				{
//					m_probabilities[(long long)row + 1][(long long)col] += smart_change;
//				}
//			}
//
//		}
//		if (col != 0)
//		{
//			if (!m_shots[(long long)row][(long long)col - 1])
//			{
//				m_probabilities[(long long)row][(long long)col - 1] += change;
//			}
//			if (m_shots[(long long)row][(long long)col - 1] == 2)
//			{
//				if (col != 9 && !m_shots[(long long)row][(long long)col + 1])
//				{
//					m_probabilities[(long long)row][(long long)col + 1] += smart_change;
//				}
//			}
//		}
//
//	}
//
//	void make_move()
//	{
//		int next_row;
//		int next_col;
//		for (int row = 0; row < 10; row++)
//		{
//			for (int col = 0; col < 10; col++)
//			{
//				if (!m_shots[row][col] && m_probabilities[row][col] != 0.0)
//				{
//					m_probabilities[row][col] = 1;
//				}
//			}
//		}
//		bool early_return = false;
//
//		bool foundhit = false;
//		for (int row = 0; row < 10; row++)
//		{
//			for (int col = 0; col < 10; col++)
//			{
//				const int8_t cell = m_shots[row][col];
//				if (cell == 2)
//				{
//					change_probs(m_shots, col, row, hit_weight, true);
//					foundhit = true;
//				}
//			}
//		}
//		if (!foundhit)
//		{
//			std::unordered_map<int, int> ships;
//			for (int i = 0; i < 4; i++)
//			{
//				ships[i + 2] = shipsleft[i];
//			}
//			for (int row = 0; row < 10; row++)
//			{
//				for (int col = 0; col < 10; col++)
//				{
//					if (!m_shots[row][col])
//					{
//						for (auto& [shiplen, shipleft] : ships)
//						{
//							if (shipleft)
//							{
//								if (validplacement(row, col, shiplen, true, m_shots))
//								{
//									for (int j = 0; j < shiplen; j++)
//									{
//										m_probabilities[row][col + j] += (shiplen + j) * unused_weight;
//									}
//								}
//								else
//								{
//									for (int j = 0; j < shiplen; j++)
//									{
//										if (!(col + j >= 0 && col + j <= 9))
//										{
//											break;
//										}
//										m_probabilities[row][col + j] -= ((shiplen - j) * unused_weight) / hit_weight;
//									}
//								}
//								if (validplacement(row, col, shiplen, false, m_shots))
//								{
//									for (int j = 0; j < shiplen; j++)
//									{
//										m_probabilities[row + j][col] += (shiplen + j) * unused_weight;
//									}
//								}
//								else
//								{
//									for (int j = 0; j < shiplen; j++)
//									{
//										if (!(row + j >= 0 && row + j <= 9))
//										{
//											break;
//										}
//										m_probabilities[row + j][col] -= ((shiplen - j) * unused_weight) / hit_weight;
//									}
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//
//		if (early_return)
//		{
//			return;
//		}
//		double max_prob = std::numeric_limits<double>::lowest();
//		std::unordered_map<double, std::pair<int, int>> candidates;
//		bool tie = false;
//		std::vector<std::pair<int, int>> tie_candidates;
//		tie_candidates.reserve(10);
//		for (int r = 0; r < 10; r++)
//		{
//			for (int c = 0; c < 10; c++)
//			{
//				if (!m_shots[r][c])
//				{
//					if (m_probabilities[r][c] > max_prob)
//					{
//						max_prob = m_probabilities[r][c];
//						tie = false;
//						tie_candidates.clear();
//					}
//					else if (m_probabilities[r][c] == max_prob)
//					{
//						tie = true;
//						tie_candidates.emplace_back(r, c);
//					}
//					candidates[m_probabilities[r][c]] = std::make_pair(r, c);
//				}
//			}
//		}
//		if (tie) {
//			std::uniform_int_distribution distro(0, (int)tie_candidates.size() - 1);
//			const std::pair<int, int> res = tie_candidates[Random::uint(0, tie_candidates.size() - 1)];
//			next_row = res.first;
//			next_col = res.second;
//		}
//		else {
//			next_row = candidates[max_prob].first;
//			next_col = candidates[max_prob].second;
//		}
//	}
//};
//
