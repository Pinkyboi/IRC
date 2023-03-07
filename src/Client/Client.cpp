# include "Client.hpp"

Client::Client(int id, struct sockaddr addr)
{
    _id = id;
    _addr = ((struct sockaddr_in *)&addr)->sin_addr;
}

Client::~Client()
{

}

int    Client::get_id() const
{
    return (_id);
}

std::string Client::get_command()
{
    std::string cmd = _commands.front().first;
    if (! _commands.front().second)
        return NULL;
    _commands.pop();
    return (cmd);
}