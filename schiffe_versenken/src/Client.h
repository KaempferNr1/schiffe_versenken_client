#pragma once
#include <deque>
#include <memory>
#include <unordered_map>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Network/Packet.hpp>

#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpSocket.hpp"
#include "Utils/json.hpp"

class Game;

struct Ship
{
    bool destroyed{ false };
    int segments_left{1};
    std::vector<std::pair<int, int>> coordinates;
    sf::Sprite sprite;
    void render(sf::RenderWindow&);
};


class Client
{
public:
    bool m_connected = false;
    std::shared_ptr<sf::TcpSocket> m_socket;
    sf::Packet m_packet_to_be_resent;
    std::deque<sf::Packet> m_packets_to_be_sent;

    Client(sf::IpAddress ip, unsigned short port);
    std::optional<nlohmann::json>  update();
    void reconnect(sf::IpAddress ip, unsigned short port);
    void connect(sf::IpAddress ip, unsigned short port);
    void disconnect();
    bool is_connected() const;
	std::optional<nlohmann::json> handle_message();

    sf::Socket::Status send_pong();

	sf::Socket::Status send_message(const std::string& message);


    void handle_partial_sends();
    void handle_not_sended();
};

