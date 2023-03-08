#include "Channel.hpp"

Channel::Channel(Client &creator ,const char * name): _name(name), _topic(NULL), _modes(0)
{
    this->add_client(creator);
    this->add_operator(creator);
}

Channel::~Channel()
{

}

void    Channel::add_client(Client& client)
{
    _clients.insert(std::pair<int, Client&>(client.get_id(), client));
}

Client& Channel::get_client(int id)
{
    return (_clients.at(id));
}

void    Channel::remove_client(Client& client)
{
    _clients.erase(client.get_id());
}

void    Channel::add_operator(Client& client)
{
    _operators.insert(std::pair<int, Client&>(client.get_id(), client));
}
