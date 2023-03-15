#include "Channel.hpp"

Channel::Channel(Client &client ,const std::string name): _name(name), _topic(""), _modes(0), _creator(client)
{
    _set_modes.insert(std::pair<char, SetMode>('t', &Channel::set_mode_t));
    _set_modes.insert(std::pair<char, SetMode>('n', &Channel::set_mode_n));
    _set_modes.insert(std::pair<char, SetMode>('s', &Channel::set_mode_s));
    _set_modes.insert(std::pair<char, SetMode>('m', &Channel::set_mode_m));
    _set_modes.insert(std::pair<char, SetMode>('k', &Channel::set_mode_k));
    _set_modes.insert(std::pair<char, SetMode>('i', &Channel::set_mode_i));
    _set_modes.insert(std::pair<char, SetMode>('v', &Channel::set_mode_v));
    _set_modes.insert(std::pair<char, SetMode>('b', &Channel::set_mode_b));
    _set_modes.insert(std::pair<char, SetMode>('o', &Channel::set_mode_o));
    _unset_modes.insert(std::pair<char, UnsetMode>('t', &Channel::unset_mode_t));
    _unset_modes.insert(std::pair<char, UnsetMode>('n', &Channel::unset_mode_n));
    _unset_modes.insert(std::pair<char, UnsetMode>('s', &Channel::unset_mode_s));
    _unset_modes.insert(std::pair<char, UnsetMode>('m', &Channel::unset_mode_m));
    _unset_modes.insert(std::pair<char, UnsetMode>('k', &Channel::unset_mode_k));
    _unset_modes.insert(std::pair<char, UnsetMode>('i', &Channel::unset_mode_i));
    _unset_modes.insert(std::pair<char, UnsetMode>('v', &Channel::unset_mode_v));
    _unset_modes.insert(std::pair<char, UnsetMode>('b', &Channel::unset_mode_b));
    _unset_modes.insert(std::pair<char, UnsetMode>('o', &Channel::unset_mode_o));
    add_client(client);
    add_operator(client);
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

std::string Channel::get_key() const
{
    return (_key);
}

int    Channel::get_clients_count() const
{
    return (_clients.size());
}

void    Channel::add_client(Client& client)
{
    _clients.insert(std::pair<int, Client&>(client.get_id(), client));
}

void    Channel::add_operator(Client& client)
{
    _operators.insert(std::pair<int, Client&>(client.get_id(), client));
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

void    Channel::add_to_invites(std::string nick)
{
    _invites.push_back(nick);
}

void    Channel::remove_from_invites(std::string nick)
{
    _invites.remove(nick);
}

std::map<int, Client&>  &Channel::get_clients(void)
{
    return _clients;
}

void    Channel::set_mode_t(std::string &mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_T;
}

void    Channel::set_mode_n(std::string &mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_N;
}

void    Channel::set_mode_s(std::string &mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_S;
}

void    Channel::set_mode_m(std::string &mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_M;
}

void    Channel::set_mode_i(std::string &mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_I;
}

void    Channel::set_mode_k(std::string &mode_argument)
{
    _key = mode_argument;
}

void    Channel::set_mode_v(std::string &mode_argument)
{
    if (std::find(_voices.begin(), _voices.end(), mode_argument) == _voices.end())
        _voices.push_back(mode_argument);
}

void    Channel::set_mode_b(std::string &mode_argument)
{
    if (std::find(_bans.begin(), _bans.end(), mode_argument) == _bans.end())
    {
        for (std::map<int, Client&>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        {
            if (it->second.get_nick() == mode_argument)
            {
                it->second.remove_channel(_name);
                remove_client(it->first);
                break;
            }
        }
        _bans.push_back(mode_argument);
    }
}

void    Channel::set_mode_o(std::string &mode_argument)
{
    for (std::map<int, Client&>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.get_nick() == mode_argument)
        {
            add_operator(it->second);
            break;
        }
    }
}

void    Channel::unset_mode_t(std::string &mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_T;
}

void    Channel::unset_mode_n(std::string &mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_N;
}

void    Channel::unset_mode_s(std::string &mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_S;
}

void    Channel::unset_mode_m(std::string &mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_M;
}

void    Channel::unset_mode_i(std::string &mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_I;
}

void    Channel::unset_mode_k(std::string &mode_argument)
{
    if (_key == mode_argument)
        _key = "";
}

void    Channel::unset_mode_v(std::string &mode_argument)
{
    _voices.remove(mode_argument);
}

void    Channel::unset_mode_b(std::string &mode_argument)
{
    _bans.remove(mode_argument);
}

void    Channel::unset_mode_o(std::string &mode_argument)
{
    if (mode_argument == _creator.get_nick())
        return ;
    for (std::map<int, Client&>::iterator it = _operators.begin(); it != _operators.end(); ++it)
    {
        if (it->second.get_nick() == mode_argument)
        {
            _operators.erase(it);
            break;
        }
    }
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
    return (std::find(_voices.begin(), _voices.end(), client.get_nick()) != _voices.end());
}

bool    Channel::is_client_banned(Client &client) const
{
    return (std::find(_bans.begin(), _bans.end(), client.get_nick()) != _bans.end());
}

bool    Channel::is_client_operator(Client &client) const
{
    return (_operators.find(client.get_id()) != _operators.end());
}

bool    Channel::is_client_invited(Client &client) const
{
    if (is_channel_invite_only())
        return (std::find(_invites.begin(), _invites.end(), client.get_nick()) != _invites.end());
    return true;
}

bool Channel::handle_modes(std::string mode, std::string mode_arg)
{
    if (mode[0] == '+')
    {
        for (size_t i = 1; i < mode.size(); i++)
        {
            if (_set_modes.find(mode[i]) != _set_modes.end())
                (this->*_set_modes[mode[i]])(mode_arg);
        }
    }
    else if (mode[0] == '-')
    {
        for (size_t i = 1; i < mode.size(); i++)
        {
            if (_unset_modes.find(mode[i]) != _unset_modes.end())
                (this->*_unset_modes[mode[i]])(mode_arg);
        }
    }
    return true;
}