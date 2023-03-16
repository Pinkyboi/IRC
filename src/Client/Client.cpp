# include "Client.hpp"

Client::Client(int id, struct sockaddr addr): _id(id), _nick(""), _username(""), _real_name(""), _pass_validity(false), _visible(true)
{
    getnameinfo(&addr, sizeof(addr), _addr, sizeof(_addr), NULL, 0, NI_NUMERICHOST);
    _status = UNREGISTERED;
    _set_modes.insert(std::make_pair('i', &Client::set_invisible));
    _unset_modes.insert(std::make_pair('i', &Client::set_visible));
}

Client::~Client()
{
    while(_commands.size() > 0)
        _commands.pop();
}

void    Client::add_command(std::string cmd)
{
    std::string     cmd_chunck;
    CircularBuffer  *cmd_buffer;
    size_t          end_index;

    while( (end_index = cmd.find("\r\n")) != std::string::npos )
    {
        cmd_chunck = cmd.substr(0, end_index);
        if (_commands.size() > 0 && _commands.back().first == false)
        {
            _commands.back().first = true;    
            _commands.back().second->push_back(cmd_chunck.c_str());
        }
        else
        {
            cmd_buffer = new CircularBuffer(MAX_COMMAND_SIZE, cmd_chunck.c_str());
            _commands.push(std::make_pair(true, cmd_buffer));
        }
        cmd = cmd.substr(end_index + 2);
    }
    if (cmd.size() > 0)
    {
        if (_commands.size() > 0 && _commands.back().first == false)
            _commands.back().second->push_back(cmd.c_str());
        else
        {
            cmd_buffer = new CircularBuffer(MAX_COMMAND_SIZE, cmd.c_str());
            _commands.push(std::make_pair(false, cmd_buffer));
        }
    }
}

std::string Client::get_command()
{
    std::string cmd;

    if (_commands.front().first == false)
        return std::string("");
    cmd = std::string(_commands.front().second->get_buffer());
    _commands.pop();
    return (cmd);
}

bool    Client::is_nick_valid(const std::string &nick)
{
    if (nick.empty() || nick.size() > 9)
        return false;
    if (nick.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_") != std::string::npos)
        return false;
    return true;
}

void    Client::set_nick(const std::string &nick)
{
    if (Client::is_nick_valid(nick))
        _nick = nick;
}

void    Client::set_username(const std::string &username)
{
    _username = username;
}

void    Client::set_real_name(const std::string &real_name)
{
    _real_name = real_name;
}

void    Client::set_mode(const std::string &mode)
{
    long number = strtol(mode.c_str(), NULL, 10);
    if (number == MODE_I)
        _visible = false;
    else if (mode.size() > 1)
        handle_modes(mode);
}

void    Client::set_invisible(void)
{
    _visible = false;
}

void    Client::set_visible(void)
{
    _visible = true;
}

void    Client::add_channel(const std::string &channel_name)
{
    _channels.push_back(channel_name);
}

void    Client::set_pass_validity(const bool validity)
{
    _pass_validity = validity;
}

void    Client::remove_channel(const std::string &channel_name)
{
    std::list<std::string>::iterator it;

    it = std::find(_channels.begin(), _channels.end(), channel_name);
    if (it != _channels.end())
        _channels.erase(it);
}

int    Client::get_id() const
{
    return (_id);
}

std::string Client::get_nick() const
{
    return (_nick);
}

std::string Client::get_username() const
{
    return (_username);
}

std::string Client::get_real_name() const
{
    return (_real_name);
}

std::list <std::string> Client::get_channels() const
{
    return (_channels);
}

bool Client::is_in_channel(std::string &c_name) const
{
    return (std::find(_channels.begin(), _channels.end(), c_name) != _channels.end());
}

bool Client::is_visible() const
{
    return (_visible);
}

bool Client::get_pass_validity() const
{
    return (_pass_validity);
}

bool Client::is_registered() const
{
    return (_status == REGISTERED);
}

int Client::get_status() const
{
    return (_status);
}

void Client::set_status(int status)
{
    _status = status;
}

void Client::update_registration()
{
    if (_pass_validity && _real_name.size() && _username.size() && _nick.size())
        _status = REGISTERED;
}

std::string Client::get_serv_id() const
{
    return _nick + "!" + _username + "@" + std::string(_addr);
}

void    Client::handle_modes(std::string mode)
{
    if (mode.empty() || (mode[0] != '+' && mode[0] != '-'))
        return;
    std::map<char, ModeFunc> mode_func = _set_modes;
    if (mode[0] == '-')
        mode_func = _unset_modes;
    for (size_t i = 1; i < mode.size(); i++)
    {
        if (mode_func.find(mode[i]) != mode_func.end())
            (this->*mode_func[mode[i]])();
    }
}