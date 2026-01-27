#include "Client.h"

#include "Game.h"
#include "SFML/Network.hpp"
#include "Utils/json.hpp"
#include "Utils/Log.h"



Client::Client(sf::IpAddress ip, unsigned short port)
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

nlohmann::json Client::update()
{
	if (!m_connected)
		return {};

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

nlohmann::json Client::handle_message()
{
	//sf::SocketSelector selector;
	//selector.add(*m_socket);
	//if (!selector.wait(sf::microseconds(1)))
	//{
	//	return {};
	//}
	sf::Packet packet;
	sf::Socket::Status status = m_socket->receive(packet);
	if (status == sf::Socket::Status::Disconnected)
	{
		LOG_INFO("disconnected");
		
		disconnect();
		m_packets_to_be_sent.clear();
		m_packet_to_be_resent.clear();
		return {};
	}
	if(status == sf::Socket::Status::NotReady)
	{
		return {};
	}
	if (status != sf::Socket::Status::Done)
	{
		LOG_INFO("message error or something");
		return {};
	}
	if (packet.getDataSize() == 0)
	{
		LOG_WARN("packet contains no data");
		return {};
	}

	std::string raw;
	packet >> raw;
	nlohmann::json message = nlohmann::json::parse(raw);
	const std::string type = message["type"];
	if (type == "ping")
	{
		//LOG_INFO("sending pong");
		status = send_pong();
		if (status == sf::Socket::Status::Disconnected)
		{
			m_packet_to_be_resent.clear();
			m_packets_to_be_sent.clear();
			disconnect();
			return {};
		}
		return {};
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
