#include "Channel.hpp"

Channel::Channel(const std::string name): _name(name), _topic("")
{
}

Channel::~Channel()
{

}

void    Channel::set_topic(std::string topic)
{
    _topic = topic;
}

std::string Channel::get_topic() const
{
    return (_topic);
}

std::string Channel::get_name() const
{
    return (_name);
}

int    Channel::get_clients_count() const
{
    return (_clients.size());
}

void    Channel::add_client(Client& client)
{
    _clients.insert(std::pair<int, Client&>(client.get_id(), client));
}

Client& Channel::get_client(int client_id)
{
    return (_clients.at(client_id));
}

void    Channel::remove_client(int client_id)
{
    _clients.erase(client_id);
}

bool    Channel::is_client(int client_id)
{
    return (_clients.find(client_id) != _clients.end());
}

std::map<int, Client&>  &Channel::get_clients(void)
{
    return _clients;
}

void    Channel::set_mode_t(std::string mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_T;
}

void    Channel::set_mode_n(std::string mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_N;
}

void    Channel::set_mode_s(std::string mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_S;
}

void    Channel::set_mode_m(std::string mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_M;
}

void    Channel::set_mode_i(std::string mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_I;
}

void    Channel::set_mode_k(std::string mode_argument)
{
    _key = mode_argument;
}

void    Channel::set_mode_v(Client &client)
{
    if (std::find(_voices.begin(), _voices.end(), client.get_serv_id()) == _voices.end())
        _voices.push_back(client.get_serv_id());
}

void    Channel::set_mode_b(Client &client)
{
    if (_operators.find(client.get_id()) == _operators.end())
    {
        if (std::find(_bans.begin(), _bans.end(), client.get_serv_id()) == _bans.end())
            _bans.push_back(client.get_serv_id());
    }
}

void    Channel::set_mode_o(Client &client)
{
    if (_operators.find(client.get_id()) == _operators.end())
        _operators.insert(std::pair<int, Client&>(client.get_id(), client));
}

void    Channel::unset_mode_t(std::string mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_T;
}

void    Channel::unset_mode_n(std::string mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_N;
}

void    Channel::unset_mode_s(std::string mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_S;
}

void    Channel::unset_mode_m(std::string mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_M;
}

void    Channel::unset_mode_i(std::string mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_I;
}

void    Channel::unset_mode_k(std::string mode_argument)
{
    if (_key == mode_argument)
        _key = "";
}

void    Channel::unset_mode_v(Client &client)
{
    _voices.remove(client.get_serv_id());
}

void    Channel::unset_mode_b(Client &client)
{
    _bans.remove(client.get_serv_id());
}

void    Channel::unset_mode_o(Client &client)
{
    _operators.erase(client.get_id());
}

bool    Channel::is_topic_lock() const
{
    return (_modes & MODE_T);
}

bool    Channel::is_channel_only() const
{
    return (_modes & MODE_N);
}

bool    Channel::is_channel_secret() const
{
    return (_modes & MODE_S);
}

bool    Channel::is_channel_moderated() const
{
    return (_modes & MODE_M);
}

bool    Channel::is_channel_invite_only() const
{
    return (_modes & MODE_I);
}

bool    Channel::is_channel_protected() const
{
    return (_key != "");
}

bool    Channel::is_client_unmute(Client &client) const
{
    return (std::find(_voices.begin(), _voices.end(), client.get_serv_id()) != _voices.end());
}

bool    Channel::is_client_banned(Client &client) const
{
    return (std::find(_bans.begin(), _bans.end(), client.get_serv_id()) != _bans.end());
}

bool    Channel::is_client_operator(Client &client) const
{
    return (_operators.find(client.get_id()) != _operators.end());
}

