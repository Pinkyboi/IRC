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
    struct pollfd *userfd = &_pfds[user_id];

    std::cout << "client disconnected: " << userfd->fd << std::endl;
    close(userfd->fd);
    memmove(userfd, userfd + 1, sizeof(struct pollfd) * (_nfds - user_id - 1));
    _clients.erase(userfd->fd);
    _operators.erase(userfd->fd);
    _nfds--;
}

void    Server::quit_cmd(int usr_id)
{
    std::string message = _parser.getMessage();

    std::cout << usr_id << " quitting" << std::endl;
    add_reply(usr_id, "QUIT", "", message);
    _clients.at(usr_id).set_status(0);
}

void    Server::privmsg_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();
    std::string message = _parser.getMessage();

    if (message.size() == 0)
        add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_NOTEXTTOSEND, MSG_NOTEXTTOSEND);
    if (args.size() == 1)
    {
        std::string name = args[0];
        if (_channels.find(name) != _channels.end())
        {
            std::map<int, Client&> &clients = _channels.at(name).get_clients();
            for (std::map<int, Client&>::iterator it = clients.begin(); it != clients.end(); it++)
                add_reply(it->first, it->second.get_nick(), RPL_PRIVMSG, message);
        }
        else if (_nicks.find(name) != _nicks.end())
            add_reply(_nicks.at(name), _clients.at(usr_id).get_nick(), RPL_PRIVMSG, message);
        else
            add_reply(usr_id, "PRIVMSG", ERR_NOSUCHNICK, MSG_NOSUCHNICK);
    }
    else if (args.size() > 1)
        add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_TOOMANYTARGETS, MSG_TOOMANYTARGETS);
    else
        add_reply(usr_id, "PRIVMSG", ERR_NORECIPIENT, MSG_NORECIPIENT);
}

void    Server::notice_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();
    std::string message = _parser.getMessage();

    if (args.size() == 1)
    {
        std::string name = args[0];
        if (_channels.find(name) != _channels.end())
        {
            std::map<int, Client&> &clients = _channels.at(name).get_clients();
            for (std::map<int, Client&>::iterator it = clients.begin(); it != clients.end(); it++)
                add_reply(it->first, it->second.get_nick(), RPL_PRIVMSG, message);
        }
        else if (_nicks.find(name) != _nicks.end())
            add_reply(_nicks.at(name), _clients.at(usr_id).get_nick(), RPL_PRIVMSG, message);
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
                std::string msg = it->first + " " + convert_to_string(it->second.get_clients_count()) + " " + it->second.get_topic();
                add_reply(usr_id, nick, RPL_LIST, msg);
            }
        }
        else if (args.size() == 1)
        {
            std::string c_name = args.at(1);
            if (_channels.find(c_name) != _channels.end())
            {
                Channel &channel = _channels.at(c_name);
                std::string msg = c_name + " " + convert_to_string(channel.get_clients_count()) + " " + channel.get_topic();
                add_reply(usr_id, nick, RPL_LIST, msg);
            }
            else
                add_reply(usr_id, nick, ERR_NOSUCHCHANNEL, args[0] + " " + MSG_NOSUCHCHANNEL);
        }
        add_reply(usr_id, nick, RPL_LISTEND, MSG_LISTEND);
    }
}

void    Server::add_reply(int usr_id, const std::string &target, const std::string &code, const std::string &msg)
{
    std::string replymsg = ":" + _severname + " " + code + " " + target + " :" + msg + CRLN;
    _replies.push(std::pair<int, std::string>(usr_id, replymsg));
}

void    Server::user_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();

    if (args.size() != 4)
        add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
    if (_clients.at(usr_id).is_registered())
        add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_ALREADYREGISTRED, MSG_ALREADYREGISTRED);
    else
    {
        std::string username = args.front();
        std::string real_name = _parser.getMessage();
        _clients.at(usr_id).set_username(username);
        _clients.at(usr_id).set_real_name(real_name);
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
        if (_nicks.find(_clients.at(usr_id).get_nick()) == _nicks.end())
        {
            if (!is_nick_used(nick))
                update_nick(usr_id, nick);
            else
                add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_NICKNAMEINUSE,  MSG_NICKNAMEINUSE);
        }
        else
        {
            if (!is_nick_used(nick))
                update_nick(usr_id, nick);
            else
                add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_NICKNAMEINUSE,  MSG_NICKNAMEINUSE);
        }
        
    }
    else
        add_reply(usr_id, "NICK", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::pass_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();

    if (_clients.at(usr_id).is_registered())
        add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_ALREADYREGISTRED, MSG_ALREADYREGISTRED);
    if (args.size() == 1)
    {
        std::string pass = args.front();
        if (pass == _pass)
            _clients.at(usr_id).set_pass_validity(true);
        //compare the passed password to the servers passowrd
    }
    else
        add_reply(usr_id, "PASS", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::oper_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();

    if (args.size() == 2)
    {
        if (_opname.empty())
            add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_NOOPERHOST, MSG_NOOPERHOST);
        Client &client = _clients.at(usr_id);
        std::string pass = args.back();
        std::string name = args.front();
        if (name == _opname && pass == _oppass) 
        {
            if (_operators.find(usr_id) == _operators.end())
            {
                _operators.insert(std::pair<int, Client&>(usr_id, client));
                add_reply(usr_id, _clients.at(usr_id).get_nick(), RPL_YOUREOPER, MSG_YOUREOPER);
            }
        }
        else if (pass != _oppass)
            add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_PASSWDMISMATCH, MSG_PASSWDMISMATCH);
    }
    else if (args.size() < 2)
        add_reply(usr_id, "OPER", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
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
                privmsg_cmd(usr_id);
                client.unset_channel();
            }
            else
                add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_NOTONCHANNEL, MSG_NOTONCHANNEL);
        }
        else
            add_reply(usr_id, c_name, ERR_NOSUCHCHANNEL, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() == 0)
        add_reply(usr_id, "PART", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
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
            Channel &target_channel =  _channels.at(c_name);
            if (is_nick_used(t_name) && target_channel.is_client(_nicks.at(t_name)))
            {
                if (is_operator(usr_id))
                {
                    if (_clients.at(usr_id).get_channel() != c_name)
                        add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_NOTONCHANNEL, MSG_NOTONCHANNEL);
                    else
                    {
                        int target_id = _nicks.at(t_name);
                        target_channel.remove_client(target_id);
                        _clients.at(target_id).unset_channel();
                        add_reply(target_id, _clients.at(usr_id).get_nick(), RPL_PRIVMSG, message);
                    }
                }
            }
            else
                add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_USERNOTINCHANNEL, MSG_USERNOTINCHANNEL);
        }
        else
            add_reply(usr_id, c_name, ERR_NOSUCHCHANNEL, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() < 2)
        add_reply(usr_id, "KICK", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
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
                add_reply(usr_id, c_name, ERR_NOTONCHANNEL, MSG_NOTONCHANNEL);
            else if (topic.empty())
            {
                if (!_channels.at(c_name).get_topic().empty())
                    add_reply(usr_id, c_name, RPL_TOPIC, _channels.at(c_name).get_topic());
                else
                    add_reply(usr_id, c_name, RPL_NOTOPIC, MSG_NOTOPIC);
            }
            else
                _channels.at(c_name).set_topic(topic);
        }
        else
            add_reply(usr_id, c_name, ERR_NOSUCHCHANNEL, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() == 0)
        add_reply(usr_id, "TOPIC", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::join_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.getArguments();

    if ( args.size() == 1 )
    {
        std::string c_name  = args.front();
        Client      &client = _clients.at(usr_id);

        if (_channels.find(c_name) == _channels.end())
        {
            _channels.insert(std::pair<std::string, Channel>(c_name, Channel(c_name)));
            _channels.at(c_name).add_client(client);
        }
        else
            _channels.at(c_name).add_client(client);
        client.set_channel(c_name);
    }
    else if (args.size() < 1)
        add_reply(usr_id, "JOIN", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
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
    {
        _parser.parse(command);
        std::string command_name = _parser.getCommand();
        if ( _commands.find(command_name) != _commands.end())
            (this->*_commands[command_name])(fd);
    }
}

void    Server::send_replies()
{
    while (!_replies.empty())
    {
        int fd = _replies.front().first;
        std::string reply = _replies.front().second;
        if(send(fd, reply.c_str(), reply.length(), 0) > 0)
            _replies.pop();
        // send_msg(reply.first, reply.second);
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
                remove_connection(i);
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