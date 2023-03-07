# include "Client.hpp"

Client::Client(int id, struct sockaddr addr)
{
    _id = id;
    _addr = ((struct sockaddr_in *)&addr)->sin_addr;
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

char *Client::get_command()
{
    char* cmd;

    if (_commands.front().first == false)
        return NULL;
    cmd = strdup(_commands.front().second->get_buffer());
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
