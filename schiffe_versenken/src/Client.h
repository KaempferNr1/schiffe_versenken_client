#pragma once
#include <deque>
#include <memory>
#include <unordered_map>
#include <SFML/Network/Packet.hpp>

#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpSocket.hpp"

class Game;

struct Ship
{
    bool destroyed;
    int segments_left;
    std::vector<std::pair<int, int>> coordinates;
};


class Client
{
public:
    bool m_connected = false;
    std::shared_ptr<sf::TcpSocket> m_socket;
    sf::Packet m_packet_to_be_resent;
    std::deque<sf::Packet> m_packets_to_be_sent;
    std::shared_ptr<Game> game;

    Client(std::shared_ptr<Game> game, sf::IpAddress ip, unsigned short port);
    void update();
    void reconnect(sf::IpAddress ip, unsigned short port);
    void connect(sf::IpAddress ip, unsigned short port);
    void disconnect();
    bool is_connected() const;

    void place_ships(int row, int col, int length, int is_horizontal) const;
    void handle_message();

    sf::Socket::Status send_pong();

	sf::Socket::Status send_message(const std::string& message);


    void handle_partial_sends();
    void handle_not_sended();
};

