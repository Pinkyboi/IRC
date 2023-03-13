# include "Client.hpp"

Client::Client(int id, struct sockaddr addr): _id(id), _nick(""), _username(""), _real_name(""), _pass_validity(false)
{
    getnameinfo(&addr, sizeof(addr), _addr, sizeof(_addr), NULL, 0, NI_NUMERICHOST);
    _status = UNREGISTERED;
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

void    Client::set_nick(const std::string &nick)
{
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
    std::remove(_channels.begin(), _channels.end(), channel_name);
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

std::list <std::string>& Client::get_channels()
{
    return (_channels);
}

bool Client::is_in_channel(std::string &c_name) const
{
    return (std::find(_channels.begin(), _channels.end(), c_name) != _channels.end());
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