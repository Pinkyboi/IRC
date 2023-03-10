#include "Server.hpp"

Server *Server::_instance = NULL;
std::string Server::_severname = "superDuperServer";
std::string Server::_opname = "MybesOPT";
std::string Server::_motd = "this is the message of the day";
std::string Server::_oppass = "123456789";

Server::~Server()
{
    for (int i = 0; i < _nfds; i++)
        close(_pfds[i].fd);
}

Server::Server(const char *port, const char *pass): _port(port), _nfds(0), _pass(pass)
{
    bool on = true;

    init_commands();
    if ((_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        throw Server::ServerException("Couldn't create socket.");
    setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    fcntl(_sockfd, F_SETFL, O_NONBLOCK);
    memset(_pfds, 0x00, sizeof(_pfds));
    _pfds[_nfds]= (struct pollfd){  .fd = _sockfd,
                                    .events = POLLIN };
    _nfds++;
}

void    Server::init_commands()
{
    _commands.insert(std::pair<std::string, cmd_func>("NICK", &Server::nick_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("USER", &Server::user_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("KICK", &Server::kick_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("JOIN", &Server::join_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("TOPIC", &Server::topic_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("LIST", &Server::list_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("PART", &Server::part_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("PASS", &Server::pass_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("OPER", &Server::oper_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("PRIVMSG", &Server::privmsg_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("QUIT", &Server::quit_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("MODE", &Server::mode_cmd));
}

bool    Server::is_nick_used(std::string& nick)
{
    return (_nicks.find(nick) != _nicks.end());
}

void    Server::setup()
{
    struct addrinfo hints;
    struct addrinfo *res, *p;

    hints = (struct addrinfo){  .ai_flags = AI_PASSIVE,
                                .ai_family = AF_INET,
                                .ai_socktype = SOCK_STREAM };
    if (getaddrinfo(NULL, this->_port, &hints, &res))
        throw Server::ServerException("Couldn't resolve host.");
    for (p = res; p; p = p->ai_next)
    {
        if (p->ai_family == AF_INET && p->ai_socktype == SOCK_STREAM)
            break;
    }
    if (p == NULL)
        throw Server::ServerException("Couldn't resolve host.");
    if (bind(this->_sockfd, p->ai_addr, sizeof(struct sockaddr_in)) < 0)
        throw Server::ServerException("Couldn't bind socket.");
    if (listen(this->_sockfd, CONN_LIMIT) < 0)
        throw Server::ServerException("Couldn't listen on socket.");
    freeaddrinfo(res);
}


bool    Server::is_operator(int client_id)
{
    return (_operators.find(client_id) != _operators.end());
}

void    Server::add_operator(Client& client)
{
    _operators.insert(std::pair<int, Client&>(client.get_id(), client));
}

void    Server::add_client(int id, struct sockaddr addr)
{
    _clients.insert(std::pair<int, Client>(id, Client(id, addr)));
}

void    Server::accept_connection()
{
    struct sockaddr addr;
    socklen_t       addrlen;
    int             new_fd;

    addr = (sockaddr){0};
    addrlen = sizeof(struct sockaddr_in);
    if ((new_fd = accept(_sockfd, &addr, &addrlen)) > 0)
    {
        std::cout << "new connection: " << new_fd << std::endl;
        add_client(new_fd, addr);
#if defined(__linux__)
        _pfds[_nfds] = (struct pollfd){ .fd = new_fd,
                                        .events = POLLIN | POLLRDHUP };
#elif defined(__APPLE__)
        _pfds[_nfds] = (struct pollfd){ .fd = new_fd,
                                        .events = POLLIN | POLLHUP };
#endif
        _nfds++;
    }
}

void    Server::remove_connection(int user_id)
{
    size_t i = 0;
    while (i < _nfds && _pfds[i].fd != user_id)
        i++;
    struct pollfd *userfd = &_pfds[i];
    std::cout << "closing connection: " << userfd->fd << std::endl;
    Client& client = _clients.at(user_id);
    std::list<std::string> &name = client.get_channels();
    for (std::list<std::string>::iterator it = name.begin(); it != name.end(); it++)
        _channels.at(*it).remove_client(user_id);
    _clients.erase(userfd->fd);
    _operators.erase(userfd->fd);
    memmove(userfd, userfd + 1, sizeof(struct pollfd) * (_nfds - i - 1));
    close(userfd->fd);
    _nfds--;
}

void    Server::quit_cmd(int usr_id)
{
    std::string message = _parser.getMessage();

    add_reply(usr_id, _severname, "QUIT", "", message);
    _clients.at(usr_id).set_status(0);
}

void    Server::privmsg_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();
    std::string message = _parser.getMessage();
    std::string s_name = _clients.at(usr_id).get_serv_id();
    if (message.size() == 0)
        add_reply(usr_id, _severname, s_name, ERR_NOTEXTTOSEND, MSG_NOTEXTTOSEND);
    if (args.size() == 1)
    {
        std::string t_name = args.front();
        if (_channels.find(t_name) != _channels.end())
        {
            if (!_channels.at(t_name).is_client_banned(_clients.at(usr_id)))
            {
                std::map<int, Client&> &clients = _channels.at(t_name).get_clients();
                for (std::map<int, Client&>::iterator it = clients.begin(); it != clients.end(); it++)
                    add_reply(it->first, s_name, it->second.get_nick(), RPL_PRIVMSG, message);
            }
            else
                add_reply(usr_id, _severname, t_name, ERR_BANNEDFROMCHAN, MSG_BANNEDFROMCHAN);
        }
        else if (_nicks.find(t_name) != _nicks.end())
            add_reply(_nicks.at(t_name), s_name, t_name, RPL_PRIVMSG, message);
        else
            add_reply(usr_id, _severname, "PRIVMSG", ERR_NOSUCHNICK, MSG_NOSUCHNICK);
    }
    else if (args.size() > 1)
        add_reply(usr_id, _severname, "PRIVMSG", ERR_TOOMANYTARGETS, MSG_TOOMANYTARGETS);
    else
        add_reply(usr_id, _severname, "PRIVMSG", ERR_NORECIPIENT, MSG_NORECIPIENT);
}


void    Server::mode_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();
    std::string message = _parser.getMessage();

    if (args.size() == 2 || args.size() == 3)
    {
        std::string c_name = args.front();
        std::string argument = "";
        std::string modes = args.at(1);
        Client &client  = _clients.at(usr_id);
        if (args.size() == 3)
            argument = args.back();
        if (_channels.find(c_name) != _channels.end())
        {
            if (_channels.at(c_name).is_client_operator(client))
                _channels.at(c_name).handle_modes(modes, argument);
            else
                add_reply(usr_id, _severname, "MODE", ERR_CHANOPRIVSNEEDED, MSG_CHANOPRIVSNEEDED);
        }
        else
            add_reply(usr_id, _severname, "MODE", ERR_NOSUCHCHANNEL, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() < 2)
        add_reply(usr_id, _severname, "MODE", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::notice_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();
    std::string message = _parser.getMessage();
    std::string s_name = _clients.at(usr_id).get_serv_id();
    if (args.size() == 1)
    {
        std::string name = args.front();
        if (_channels.find(name) != _channels.end())
        {
            std::map<int, Client&> &clients = _channels.at(name).get_clients();
            for (std::map<int, Client&>::iterator it = clients.begin(); it != clients.end(); it++)
                add_reply(it->first, s_name, it->second.get_nick(), RPL_PRIVMSG, message);
        }
        else if (_nicks.find(name) != _nicks.end())
            add_reply(_nicks.at(name), _severname, s_name, RPL_PRIVMSG, message);
    }
}

std::string convert_to_string(int number)
{
    std::stringstream ss;

    ss << number;
    return ss.str();
}

void    Server::list_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();

    if (args.size() == 0 || args.size() == 1)
    {
        std::string nick = _clients.at(usr_id).get_nick();
        if (args.size() == 0)
        {
            for (std::map<const std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
            {
                if (it->second.is_channel_secret() == false)
                {
                    std::string msg = it->first + " " + convert_to_string(it->second.get_clients_count()) + " " + it->second.get_topic();
                    add_reply(usr_id, _severname, nick, RPL_LIST, msg);
                }
            }
        }
        else if (args.size() == 1)
        {
            std::string c_name = args.at(1);
            if (_channels.find(c_name) != _channels.end())
            {
                Channel &channel = _channels.at(c_name);
                std::string msg = c_name + " " + convert_to_string(channel.get_clients_count()) + " " + channel.get_topic();
                add_reply(usr_id, _severname, nick, RPL_LIST, msg);
            }
            else
                add_reply(usr_id, _severname, nick, ERR_NOSUCHCHANNEL, args[0] + " " + MSG_NOSUCHCHANNEL);
        }
        add_reply(usr_id, _severname, nick, RPL_LISTEND, MSG_LISTEND);
    }
}

void    Server::add_reply(int usr_id, const std::string &sender, const std::string &target, const std::string &code, const std::string &msg)
{
    std::string replymsg = ":" + sender + " " + code + " " + target + " :" + msg + CRLN;
    _replies.push(std::pair<int, std::string>(usr_id, replymsg));
}

void    Server::user_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();
    std::string real_name = _parser.getMessage();

    if (args.size() != 3 || real_name.empty())
        add_reply(usr_id, _severname, _clients.at(usr_id).get_nick(), ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
    if (_clients.at(usr_id).is_registered())
        add_reply(usr_id, _severname, _clients.at(usr_id).get_nick(), ERR_ALREADYREGISTRED, MSG_ALREADYREGISTRED);
    else
    {
        std::string username = args.front();
        _clients.at(usr_id).set_username(username);
        _clients.at(usr_id).set_real_name(real_name);
        _clients.at(usr_id).update_registration();
    }
}

void    Server::update_nick(int usr_id, std::string newnick)
{
    std::string oldnick = _clients.at(usr_id).get_nick();
    _clients.at(usr_id).set_nick(newnick);
    _nicks.erase(oldnick);
    _nicks.insert(std::pair<std::string, int>(newnick, usr_id));
}

void    Server::add_nick(int usr_id, std::string nick)
{
    _clients.at(usr_id).set_nick(nick);
    _nicks.insert(std::pair<std::string, int>(nick, usr_id));
}

void    Server::nick_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();

    if ( args.size() == 1 )
    {
        std::string nick  = args.front();
        Client &client = _clients.at(usr_id);
        if ( is_nick_used(nick) ==  false )
        {
            if (client.is_registered())
                update_nick(usr_id, nick);
            else
            {
                client.set_nick(nick);
                client.update_registration();
                add_nick(usr_id, nick);
            }
        }
        else
            add_reply(usr_id, _severname, client.get_nick(), ERR_NICKNAMEINUSE,  MSG_NICKNAMEINUSE);
        
    }
    else
        add_reply(usr_id, _severname, "NICK", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::pass_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();

    if ( _clients.at(usr_id).is_registered() )
        add_reply(usr_id, _severname, _clients.at(usr_id).get_nick(), ERR_ALREADYREGISTRED, MSG_ALREADYREGISTRED);
    if ( args.size() == 1 )
    {
         _clients.at(usr_id).set_pass_validity(args.front() == _pass);
         _clients.at(usr_id).update_registration();
    }
    else
        add_reply(usr_id, _severname, "PASS", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::oper_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();

    if (args.size() == 2)
    {
        if (_opname.empty())
            add_reply(usr_id, _severname, _clients.at(usr_id).get_nick(), ERR_NOOPERHOST, MSG_NOOPERHOST);
        Client &client = _clients.at(usr_id);
        std::string pass = args.back();
        std::string name = args.front();
        if (name == _opname && pass == _oppass) 
        {
            if (_operators.find(usr_id) == _operators.end())
            {
                _operators.insert(std::pair<int, Client&>(usr_id, client));
                add_reply(usr_id, _severname, _clients.at(usr_id).get_nick(), RPL_YOUREOPER, MSG_YOUREOPER);
            }
        }
        else if (pass != _oppass)
            add_reply(usr_id, _severname, _clients.at(usr_id).get_nick(), ERR_PASSWDMISMATCH, MSG_PASSWDMISMATCH);
    }
    else if (args.size() < 2)
        add_reply(usr_id, _severname, "OPER", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::part_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();

    if (args.size() == 1)
    {  
        std::string c_name  = args.front();
        std::string message = _parser.getMessage();
        Client      &client = _clients.at(usr_id);

        if (_channels.find(c_name) != _channels.end())
        {
            if (_channels.at(c_name).is_client(usr_id))
            {
                _channels.at(c_name).remove_client(usr_id);
                if (message != "")
                    privmsg_cmd(usr_id);
                client.remove_channel(c_name);
            }
            else
                add_reply(usr_id, _severname, _clients.at(usr_id).get_nick(), ERR_NOTONCHANNEL, MSG_NOTONCHANNEL);
        }
        else
            add_reply(usr_id, _severname, c_name, ERR_NOSUCHCHANNEL, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() == 0)
        add_reply(usr_id, _severname, "PART", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::kick_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();

    if ( args.size() == 2 )
    {
        std::string c_name   = args.front();
        std::string t_name   = args.at(1);
        std::string message  = _parser.getMessage();
        if (_channels.find(c_name) != _channels.end())
        {
            Channel &target_channel =   _channels.at(c_name);
            Client  &sender         =   _clients.at(usr_id);
            if (is_nick_used(t_name) && target_channel.is_client(_nicks.at(t_name)))
            {
                if (is_operator(usr_id))
                {
                    if (sender.is_in_channel(c_name))
                        add_reply(usr_id, _severname, sender.get_nick(), ERR_NOTONCHANNEL, MSG_NOTONCHANNEL);
                    else
                    {
                        int target_id = _nicks.at(t_name);
                        target_channel.remove_client(target_id);
                        _clients.at(target_id).remove_channel(c_name);
                        add_reply(target_id, sender.get_serv_id(), t_name, RPL_PRIVMSG, message);
                    }
                }
            }
            else
                add_reply(usr_id, _severname, sender.get_nick(), ERR_USERNOTINCHANNEL, MSG_USERNOTINCHANNEL);
        }
        else
            add_reply(usr_id, _severname, c_name, ERR_NOSUCHCHANNEL, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() < 2)
        add_reply(usr_id, _severname, "KICK", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::topic_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();

    if ( args.size() == 1 )
    {
        std::string c_name   = args.front();
        std::string topic    = _parser.getMessage();
        if (_channels.find(c_name) != _channels.end())
        {
            Channel &channel = _channels.at(c_name);
            if (!channel.is_client(usr_id))
                add_reply(usr_id, _severname, c_name, ERR_NOTONCHANNEL, MSG_NOTONCHANNEL);
            else if (topic.empty())
            {
                if (!_channels.at(c_name).get_topic().empty())
                    add_reply(usr_id, _severname, c_name, RPL_TOPIC, _channels.at(c_name).get_topic());
                else
                    add_reply(usr_id, _severname, c_name, RPL_NOTOPIC, MSG_NOTOPIC);
            }
            else
                _channels.at(c_name).set_topic(topic);
        }
        else
            add_reply(usr_id, _severname, c_name, ERR_NOSUCHCHANNEL, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() == 0)
        add_reply(usr_id, _severname, "TOPIC", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::join_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();

    if ( args.size() == 1 || args.size() == 2 )
    {
        std::string c_name  = args.front();
        std::string key     = "";
        Client      &client = _clients.at(usr_id);

        if (args.size() == 2)
            key = args.at(1);
        if (_channels.find(c_name) == _channels.end())
            _channels.insert(std::pair<std::string, Channel>(c_name, Channel(client, c_name)));
        else
        {
            if (_channels.at(c_name).get_key() == key)
            {
                if (!_channels.at(c_name).is_client_banned(client))
                    _channels.at(c_name).add_client(client);
                else
                    add_reply(usr_id, _severname, c_name, ERR_BANNEDFROMCHAN, MSG_BANNEDFROMCHAN);
            }
            else
                add_reply(usr_id, _severname, c_name, ERR_BADCHANNELKEY, MSG_BADCHANNELKEY);
        }
        client.add_channel(c_name);
    }
    else if (args.size() < 1)
        add_reply(usr_id, _severname, "JOIN", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::handle_commands(int fd, std::string &command)
{
    _parser.parse(command);
    std::string command_name = _parser.getCommand();
    if ( _commands.find(command_name) != _commands.end())
    {
        Client &client = _clients.at(fd);
        if ( client.get_status() == Client::UNREGISTERED )
        {
            if ( command_name == "PASS" || command_name == "USER"
                    || command_name == "NICK" || command_name == "QUIT" )
                (this->*_commands[command_name])(fd);
            else
                add_reply(fd, _severname, client.get_nick(), ERR_NOTREGISTERED, MSG_NOTREGISTERED);
        }
        else if ( client.get_status() == Client::REGISTERED )
        {
            if ( command_name == "PASS" ||  command_name == "USER" )
                add_reply(fd, _severname, client.get_nick(), ERR_ALREADYREGISTRED, MSG_ALREADYREGISTRED);
            else
                (this->*_commands[command_name])(fd);
        }
    }
}

void    Server::print_msg(int fd)
{
    static char msg_buffer[MAX_COMMAND_SIZE];
    size_t      msg_len;
    std::string command;

    if ( (msg_len = recv(fd, msg_buffer, MAX_COMMAND_SIZE, MSG_DONTWAIT)) > 0)
    {
        msg_buffer[msg_len] = '\0';
        _clients.at(fd).add_command(std::string(msg_buffer));
    }
    while((command = _clients.at(fd).get_command()) != "")
        handle_commands(fd, command);
}

void    Server::send_replies()
{
    while (!_replies.empty())
    {
        int fd = _replies.front().first;
        std::string reply = _replies.front().second;
        if (send(fd, reply.c_str(), reply.length(), 0) > 0)
            _replies.pop();
        if (_clients.at(fd).get_status() == Client::DOWN)
            remove_connection(fd);
    }
}

void    Server::start()
{
    while (true)
    {
        if (poll(_pfds, _nfds, -1) < 0)
            continue;
        for (int i = 0; i < _nfds; i++)
        {
#if defined(__linux__)
            if (_pfds[i].revents & POLLRDHUP)
#elif defined(__APPLE__)
            if (_pfds[i].revents & POLLHUP)
#endif
                remove_connection(_pfds[i].fd);
            else if (_pfds[i].revents & POLLIN)
            {
                if (_pfds[i].fd != _sockfd)
                    print_msg(_pfds[i].fd);
                else
                    accept_connection();
            }
        }
        send_replies();
    }
}

Server* Server::getInstance()
{
    return Server::_instance;
};

void    Server::deleteInstance()
{
    if (Server::_instance != NULL)
    {
        delete Server::_instance;
        Server::_instance = NULL;
    }
}

void     Server::initServer(const char *port, const char *pass)
{
    if (Server::_instance == NULL)
        Server::_instance = new Server(port, pass);
    else
        throw Server::ServerException("Server already initialized.");
};