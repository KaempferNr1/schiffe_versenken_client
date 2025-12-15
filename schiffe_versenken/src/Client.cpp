#include "Client.h"

#include "Game.h"
#include "SFML/Network.hpp"
#include "Utils/json.hpp"
#include "Utils/Log.h"

Client::Client(std::shared_ptr<Game> game, sf::IpAddress ip, unsigned short port): game(game)
{
	m_socket = std::make_shared<sf::TcpSocket>();
	m_socket->setBlocking(true);
	const sf::Socket::Status status = m_socket->connect(ip, port, sf::microseconds(10));
	if (status == sf::Socket::Status::Done)
	{
		m_connected = true;
	}
	m_socket->setBlocking(false);
}

std::optional<nlohmann::json> Client::update()
{
	if (!m_connected)
		return std::nullopt;

	handle_partial_sends();
	handle_not_sended();
	return handle_message();
}

void Client::reconnect(sf::IpAddress ip, unsigned short port)
{
	m_socket->setBlocking(true);
	const sf::Socket::Status status = m_socket->connect(ip, port,sf::microseconds(10));
	if (status == sf::Socket::Status::Done)
	{
		m_connected = true;
	}
	else
	{
		m_connected = false;
	}
	m_socket->setBlocking(false);
}

void Client::connect(sf::IpAddress ip, unsigned short port)
{
	if(!m_connected)
	{
		reconnect(ip, port);
	}
}

void Client::disconnect()
{
	m_socket->disconnect();
	m_connected = false;
}

bool Client::is_connected() const
{
	return m_connected;
}

void Client::place_ships(int row, int col, int length, int is_horizontal) const
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
	std::array<std::array<int8_t, 10>, 10>& ship_map = game->m_ship_map;
	std::vector<Ship>& ships = game->m_ships;
	Ship ship;
	ship.destroyed = false;
	for (int offset_in_col = 0; offset_in_col < amount_of_cols; ++offset_in_col)
	{
		for (int offset_in_row = 0; offset_in_row < amount_of_rows; ++offset_in_row)
		{
			const int current_row = row + offset_in_row;
			const int current_col = col + offset_in_col;
			ship.coordinates.emplace_back(current_row, current_col);
			ship_map[current_row][current_col] = static_cast<uint8_t>(ships.size()) + 1u;
		}
	}
	ship.segments_left = length;
	ships.push_back(ship);
}

std::optional<nlohmann::json> Client::handle_message()
{
	sf::SocketSelector selector;
	selector.add(*m_socket);
	if (!selector.wait(sf::microseconds(10)))
	{
		return std::nullopt;
	}
	sf::Packet packet;
	sf::Socket::Status status = m_socket->receive(packet);
	if (status == sf::Socket::Status::Disconnected)
	{
		LOG_INFO("disconnected");
		
		disconnect();
		m_packets_to_be_sent.clear();
		m_packet_to_be_resent.clear();
		return std::nullopt;
	}
	if (status != sf::Socket::Status::Done)
	{
		LOG_INFO("message error or something");
		return std::nullopt;
	}
	if (packet.getDataSize() == 0)
	{
		LOG_WARN("packet contains no data");
		return std::nullopt;
	}

	std::string raw;
	packet >> raw;
	nlohmann::json message = nlohmann::json::parse(raw);
	std::string type = message["type"];
	if (type == "ping")
	{
		//LOG_INFO("sending pong");
		status = send_pong();
		if (status == sf::Socket::Status::Disconnected)
		{
			m_packet_to_be_resent.clear();
			m_packets_to_be_sent.clear();
			disconnect();
			return std::nullopt;
		}
		return std::nullopt;
	}
	if(type == "success")
	{
		const int row = message["row"];
		const int col = message["col"];
		const int length = message["length"];
		const int is_horizontal = message["is_horizontal"];
		place_ships(row, col, length, is_horizontal);
	}
	else if(type == "shot")
	{

		if (message["kind"] != "receive") 
		{
			const int row = message["row"];
			const int col = message["col"];
			game->m_shots[row][col] = message["kind"] == "miss" ? 1 : 2;
		}
		else
		{
			const int row = message["row"];
			const int col = message["col"];
			const uint8_t cell = game->m_ship_map[row][col];

			if (cell > 0)
			{
				game->m_ship_map[row][col] = -1;
				if(--game->m_ships[cell - 1].segments_left == 0)
				{
					game->m_ships[cell - 1].destroyed = true;
				}
			}
			else
			{
				game->m_ship_map[row][col] = -2;
			}
		}
	}
	LOG_INFO("{}", message.dump());
	return message;
}

sf::Socket::Status Client::send_pong()
{
	nlohmann::json msg;
	msg["type"] = "pong";
	return send_message(msg.dump());
}

sf::Socket::Status Client::send_message(const std::string& message)
{
	sf::Packet packet;
	packet << message;
	if (m_packet_to_be_resent.getDataSize() != 0)
	{
		m_packets_to_be_sent.push_back(std::move(packet));
		return sf::Socket::Status::NotReady;
	}
	const sf::Socket::Status status = m_socket->send(packet);
	if (status == sf::Socket::Status::Partial)
	{
		LOG_INFO("partial send");
		m_packet_to_be_resent = std::move(packet);
	}

	return status;
}

void Client::handle_partial_sends()
{
	if (m_packet_to_be_resent.getDataSize() == 0)
		return;
	const sf::Socket::Status status = m_socket->send(m_packet_to_be_resent);

	if (status == sf::Socket::Status::Done)
	{
		m_packet_to_be_resent.clear();
		return;
	}

	if (status == sf::Socket::Status::Disconnected)
	{
		m_packet_to_be_resent.clear();
		m_packets_to_be_sent.clear();
		disconnect();
	}
}

void Client::handle_not_sended()
{

	if (m_packet_to_be_resent.getDataSize() != 0)
	{
		return;
	}
	while (!m_packets_to_be_sent.empty())
	{
		auto& packet = m_packets_to_be_sent.front();
		const sf::Socket::Status status = m_socket->send(packet);
		if (status == sf::Socket::Status::Disconnected)
		{
			m_packet_to_be_resent.clear();
			m_packets_to_be_sent.clear();
			disconnect();
			break;
		}
		else if (status == sf::Socket::Status::Partial)
		{
			LOG_INFO("partial send");
			m_packet_to_be_resent.clear();
			m_packet_to_be_resent = std::move(packet);
			m_packets_to_be_sent.pop_front();
			break;
		}
		else if (status == sf::Socket::Status::Done)
		{
			m_packets_to_be_sent.pop_front();
		}
		else
		{
			break;
		}
	}


}
