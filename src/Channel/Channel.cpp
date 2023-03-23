#include "Channel.hpp"

Channel::Channel(Client &client ,const std::string name): _name(name), _topic(""), _owner(client), _modes(MODE_N | MODE_T), _key(""), _limit(0)
{
    _set_modes.insert(std::pair<char, ModeFunc>('t', &Channel::set_mode_t));
    _set_modes.insert(std::pair<char, ModeFunc>('n', &Channel::set_mode_n));
    _set_modes.insert(std::pair<char, ModeFunc>('s', &Channel::set_mode_s));
    _set_modes.insert(std::pair<char, ModeFunc>('m', &Channel::set_mode_m));
    _set_modes.insert(std::pair<char, ModeFunc>('k', &Channel::set_mode_k));
    _set_modes.insert(std::pair<char, ModeFunc>('i', &Channel::set_mode_i));
    _set_modes.insert(std::pair<char, ModeFunc>('v', &Channel::set_mode_v));
    _set_modes.insert(std::pair<char, ModeFunc>('b', &Channel::set_mode_b));
    _set_modes.insert(std::pair<char, ModeFunc>('o', &Channel::set_mode_o));
    _set_modes.insert(std::pair<char, ModeFunc>('l', &Channel::set_mode_l));
    _unset_modes.insert(std::pair<char, ModeFunc>('t', &Channel::unset_mode_t));
    _unset_modes.insert(std::pair<char, ModeFunc>('n', &Channel::unset_mode_n));
    _unset_modes.insert(std::pair<char, ModeFunc>('s', &Channel::unset_mode_s));
    _unset_modes.insert(std::pair<char, ModeFunc>('m', &Channel::unset_mode_m));
    _unset_modes.insert(std::pair<char, ModeFunc>('k', &Channel::unset_mode_k));
    _unset_modes.insert(std::pair<char, ModeFunc>('i', &Channel::unset_mode_i));
    _unset_modes.insert(std::pair<char, ModeFunc>('v', &Channel::unset_mode_v));
    _unset_modes.insert(std::pair<char, ModeFunc>('b', &Channel::unset_mode_b));
    _unset_modes.insert(std::pair<char, ModeFunc>('o', &Channel::unset_mode_o));
    _unset_modes.insert(std::pair<char, ModeFunc>('l', &Channel::unset_mode_l));
    add_client(client);
    if (_name[0] != '+')
        add_operator(client);
}

Channel::~Channel()
{
}

std::string Channel::get_valid_channel_name(std::string name)
{
    if (name.empty())
        return "";
    if (name[0] != '#' && name[0] != '&' && name[0] != '+')
        return "";
    if (name.find_first_of(",+-!?.=*@~") != std::string::npos)
        return "";
    return name.substr(0, 50);
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

std::string Channel::get_name_with_topic() const
{
    return (_name + " :" + _topic);
}

std::string Channel::get_key() const
{
    return (_key);
}

std::string Channel::get_owner_nick() const
{
    return (_owner.get_nick());
}

std::string Channel::get_member_prefix(Client &client)
{
    std::string name;

    if (is_client_owner(client))
        name = "~";
    else if (is_client_operator(client))
        name = "@";
    else if (is_channel_moderated() && is_client_unmute(client))
        name = "+";
    return (name); 
}


std::string Channel::get_mode_args() const
{
    std::string args = "";
    if (_key.size())
        args += " " + _key;
    if (_limit)
    {
        std::stringstream ss;
        ss << _limit;
        args += " " + ss.str();
    }
    return args;
}

std::string Channel::get_modes() const
{
    if (_modes == 0)
        return "";
    std::string modes = "";
    modes += "+";
    if (_modes & MODE_T)
        modes += "t";
    if (_modes & MODE_N)
        modes += "n";
    if (_modes & MODE_S)
        modes += "s";
    if (_modes & MODE_M)
        modes += "m";
    if (_modes & MODE_I)
        modes += "i";
    if (_modes & MODE_K)
        modes += "k";
    if (_modes & MODE_L)
        modes += "l";
    return modes;
}

std::string Channel::get_modes_with_args() const
{
    return get_modes() + get_mode_args();
}

size_t    Channel::get_clients_count() const
{
    return (_clients.size());
}

size_t    Channel::get_present_count() const
{
    return (_present.size());
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
    _operators.erase(client_id);
    _invites.erase(client_id);
    _voices.erase(client_id);
    _present.erase(client_id);
}

void    Channel::part_client(int client_id)
{
    _present.erase(client_id);
}

void    Channel::add_client(Client& client)
{
    int client_id = client.get_id();
    if (_clients.find(client_id) == _clients.end())
        _clients.insert(std::pair<int, Client&>(client.get_id(), client));
    if (_present.find(client_id) == _present.end())
        _present.insert(std::pair<int, Client&>(client_id, client));
}

bool    Channel::is_client(int client_id)
{
    return (_clients.find(client_id) != _clients.end());
}


void    Channel::add_to_invites(Client &client)
{
    _invites.insert(std::pair<int, Client&>(client.get_id(), client));
}

void    Channel::remove_from_invites(int client_id)
{
    _invites.erase(client_id);
}

std::map<int, Client&>  &Channel::get_present_clients(void)
{
    return _present;
}

std::map<int, Client&>  &Channel::get_clients(void)
{
    return _clients;
}

bool    Channel::set_mode_t(std::queue<std::string> &mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_T;
    return true;
}

bool    Channel::set_mode_n(std::queue<std::string> &mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_N;
    return true;
}

bool    Channel::set_mode_s(std::queue<std::string> &mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_S;
    return true;
}

bool    Channel::set_mode_m(std::queue<std::string> &mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_M;
    return true;
}

bool    Channel::set_mode_i(std::queue<std::string> &mode_argument)
{
    (void)mode_argument;
    _modes |= MODE_I;
    return true;
}

bool    Channel::set_mode_k(std::queue<std::string> &mode_argument)
{
    if (mode_argument.empty())
        return false;
    std::string mode_arg = mode_argument.front();
    mode_argument.pop();
    _key = mode_arg;
    _modes |= MODE_K;
    return true;
}

bool    Channel::set_mode_v(std::queue<std::string> &mode_argument)
{
    if (mode_argument.empty())
        return false;
    std::string mode_arg = mode_argument.front();
    mode_argument.pop();
    for (std::map<int, Client&>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.get_nick() == mode_arg)
            _voices.insert(std::pair<int, Client&>(it->second.get_id(), it->second));
    }
    return false;
}

bool    Channel::set_mode_b(std::queue<std::string> &mode_argument)
{
    if (mode_argument.empty())
        return false;
    std::string mode_arg = mode_argument.front();
    mode_argument.pop();
    if (std::find(_bans.begin(), _bans.end(), mode_arg) == _bans.end())
    {
        for (std::map<int, Client&>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        {
            if (it->second.get_nick() == mode_arg)
            {
                it->second.remove_channel(_name);
                remove_client(it->first);
                break;
            }
        }
        _bans.push_back(mode_arg);
    }
    return false;
}

bool    Channel::set_mode_o(std::queue<std::string> &mode_argument)
{
    if (mode_argument.empty())
        return false;
    std::string mode_arg = mode_argument.front();
    mode_argument.pop();
    for (std::map<int, Client&>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.get_nick() == mode_arg)
        {
            add_operator(it->second);
            break;
        }
    }
    return false;
}

bool    Channel::set_mode_l(std::queue<std::string> &mode_argument)
{
    long input_limit;

    if (mode_argument.empty())
        return false;
    std::string mode_arg = mode_argument.front();
    mode_argument.pop();
    input_limit = strtol(mode_arg.c_str(), NULL, 10);
    if (input_limit < 0)
        return false;
    if (_clients.size() <= (unsigned long)input_limit)
    {
        _limit = input_limit;
        _modes |= MODE_L;
        return true;
    }
    return false;
}

bool    Channel::unset_mode_t(std::queue<std::string> &mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_T;
    return true;
}

bool    Channel::unset_mode_n(std::queue<std::string> &mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_N;
    return true;
}

bool    Channel::unset_mode_s(std::queue<std::string> &mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_S;
    return true;
}

bool    Channel::unset_mode_m(std::queue<std::string> &mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_M;
    return true;
}

bool    Channel::unset_mode_i(std::queue<std::string> &mode_argument)
{
    (void)mode_argument;
    _modes &= ~MODE_I;
    return true;
}

bool    Channel::unset_mode_k(std::queue<std::string> &mode_argument)
{
    if (mode_argument.empty())
        return false;
    std::string mode_arg = mode_argument.front();
    mode_argument.pop();
    if (_key != mode_arg)
        return false;
    _key = "";
    _modes &= ~MODE_K;
    return true;
}

bool    Channel::unset_mode_v(std::queue<std::string> &mode_argument)
{
    if (mode_argument.empty())
        return false;
    std::string mode_arg = mode_argument.front();
    mode_argument.pop();
    if (mode_arg == _owner.get_nick())
        return false;
    for (std::map<int, Client&>::iterator it = _voices.begin(); it != _voices.end(); ++it)
    {
        if (it->second.get_nick() == mode_arg)
        {
            if (is_client_operator(it->second) == false)
                _voices.erase(it);
            break;
        }
    }
    return false;
}

bool    Channel::unset_mode_b(std::queue<std::string> &mode_argument)
{
    if (mode_argument.empty())
        return false;
    std::string mode_arg = mode_argument.front();
    mode_argument.pop();
    _bans.remove(mode_arg);
    return false;
}

bool    Channel::unset_mode_o(std::queue<std::string> &mode_argument)
{
    if (mode_argument.empty())
        return false;
    std::string mode_arg = mode_argument.front();
    mode_argument.pop();
    if (mode_arg == _owner.get_nick())
        return false;
    for (std::map<int, Client&>::iterator it = _operators.begin(); it != _operators.end(); ++it)
    {
        if (it->second.get_nick() == mode_arg)
        {
            _operators.erase(it);
            break;
        }
    }
    return false;
}

bool    Channel::unset_mode_l(std::queue<std::string> &mode_argument)
{
    (void)mode_argument;
    _limit = 0;
    _modes &= ~MODE_L;
    return true;
}

bool    Channel::is_topic_lock() const
{
    return (_modes & MODE_T);
}

bool    Channel::is_channel_client_only() const
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
    return (_modes & MODE_K);
}

bool    Channel::is_key_valid(const std::string &key) const
{
    return (_key == key);
}
bool    Channel::is_client_unmute(Client &client) const
{
    if (is_channel_moderated() == false || is_client_operator(client))
        return true;
    return (_voices.find(client.get_id()) != _voices.end());
}

bool    Channel::is_client_banned(Client &client) const
{
    return (std::find(_bans.begin(), _bans.end(), client.get_nick()) != _bans.end());
}

bool    Channel::is_client_operator(Client &client) const
{
    return (_operators.find(client.get_id()) != _operators.end());
}

bool    Channel::is_client_present(Client &client) const
{
    return (_present.find(client.get_id()) != _present.end());
}

bool   Channel::is_client_owner(Client &client) const
{
    return (_owner.get_id() == client.get_id());
}

bool    Channel::is_client_invited(Client &client) const
{
    if (is_channel_invite_only())
    {
        if (_clients.find(client.get_id()) != _clients.end())
            return true;
        return (_invites.find(client.get_id()) != _invites.end());
    }
    return true;
}

bool    Channel::is_there_space() const
{
    if (_limit > 0)
        return (_clients.size() < _limit);
    return true;
}

std::string Channel::handle_modes(std::string mode, std::queue<std::string>& mode_arg)
{
    std::map<char, ModeFunc>    mode_func = _set_modes;
    std::string                 mode_used = "+";
    std::string                 arg_used = "";
    std::string                 current_arg = "";
    if (mode.empty() || (mode[0] != '+' && mode[0] != '-'))
        return "";
    if (mode[0] == '-')
    {
        mode_func = _unset_modes;
        mode_used = "-";
    }
    for (size_t i = 1; i < mode.size(); i++)
    {
        if (mode_func.find(mode[i]) != mode_func.end())
        {
            if (mode_used.find(mode[i]) == std::string::npos)
            {
                if (mode_arg.size())
                    current_arg = mode_arg.front();
                if ((this->*mode_func[mode[i]])(mode_arg))
                    if (mode[i] == 'k' || mode[i] == 'l')
                        arg_used += " " + current_arg;
                mode_used += mode[i];
            }
        }
    }
    return mode_used + arg_used;
}