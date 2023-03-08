#include "Channel.hpp"

Channel::Channel(Client &creator , const std::string name): _name(name), _topic(NULL), _modes(0)
{
    this->add_client(creator);
    this->add_operator(creator);
}

Channel::~Channel()
{

}

void    Channel::set_topic(std::string topic)
{
    _topic = topic;
}

void    Channel::set_nick(int client_id, std::string nick)
{
    _nicks.erase(_clients.at(client_id).get_nick());
    _nicks.insert(std::pair<std::string, int>(nick, client_id));
    _clients.at(client_id).set_nick(nick);
}

std::string Channel::get_topic() const
{
    return (_topic);
}

std::string Channel::get_name() const
{
    return (_name);
}

void    Channel::add_client(Client& client)
{
    _clients.insert(std::pair<int, Client&>(client.get_id(), client));
    _nicks.insert(std::pair<std::string, int>(client.get_nick(), client.get_id()));
}

Client& Channel::get_client(int client_id)
{
    return (_clients.at(client_id));
}

int    Channel::get_client_id(std::string& nick)
{
    return (_nicks.at(nick));
}

void    Channel::remove_client(int client_id)
{
    _clients.erase(client_id);
}

bool    Channel::is_client(int client_id)
{
    return (_clients.find(client_id) != _clients.end());
}

bool    Channel::is_operator(int client_id)
{
    return (_operators.find(client_id) != _operators.end());
}

bool    Channel::is_nick_used(std::string& nick)
{
    return (_nicks.find(nick) != _nicks.end());
}

void    Channel::add_operator(Client& client)
{
    _operators.insert(std::pair<int, Client&>(client.get_id(), client));
}
