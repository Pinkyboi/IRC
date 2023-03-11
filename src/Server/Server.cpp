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
    _commands.insert(std::pair<std::string, cmd_func>("PART", &Server::part_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("PASS", &Server::pass_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("OPER", &Server::oper_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("MSG", &Server::msg_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("PRIVMSG", &Server::privmsg_cmd));
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

void    Server::privmsg_cmd(int usr_id, std::vector<std::string> &args)
{

}

void    Server::msg_cmd(int usr_id, std::vector<std::string> &args)
{
    
}

void    Server::notice_cmd(int usr_id, std::vector<std::string> &args)
{
    
}

void    Server::list_cmd(int usr_id, std::vector<std::string> &args)
{
    // send list start reply to client
    // for each server present list it with the number of users
    // if not argument specified send list of all channels with number of users
    // end end of list reply
}

void    Server::add_reply(int usr_id, const std::string &target, const std::string &code, const std::string &msg)
{
    std::string replymsg = ":" + _severname + " " + code + " " + target + " :" + msg + CRLN;
    _replies.push(std::pair<int, std::string>(usr_id, replymsg));
}

void    Server::user_cmd(int usr_id, std::vector<std::string> &args)
{
    if (args.size() != 4)
        add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
    if (_clients.at(usr_id).is_registered())
        add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_ALREADYREGISTRED, MSG_ALREADYREGISTRED);
    else
    {
        std::string username = args.front();
        std::string real_name = args.back();
        _clients.at(usr_id).set_username(username);
        _clients.at(usr_id).set_real_name(real_name);
    }
}

void    Server::nick_cmd(int usr_id, std::vector<std::string> &args)
{
    if ( args.size() == 1 )
    {
        std::string nick  = args.front();
        if (_clients.at(usr_id).get_channel() == "")
        {
            for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
            {
                if (it->second.get_nick() == nick)
                    add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_NICKNAMEINUSE,  MSG_NICKNAMEINUSE); 
            }
            _clients.at(usr_id).set_nick(nick);
        }
        else
        {
            std::cout << _clients.at(usr_id).get_channel() << std::endl;
            Channel &current_channel = _channels.at(_clients.at(usr_id).get_channel());
            if (!current_channel.is_nick_used(nick))
                current_channel.update_nick(usr_id, nick);
            else
                add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_NICKNAMEINUSE,  MSG_NICKNAMEINUSE);
        }
    }
    else
        add_reply(usr_id, "NICK", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::pass_cmd(int usr_id, std::vector<std::string> &args)
{
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

void    Server::oper_cmd(int usr_id, std::vector<std::string> &args)
{
    if (args.size() == 2)
    {
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
    }
    else if (args.size() < 2)
        add_reply(usr_id, "OPER", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::part_cmd(int usr_id, std::vector<std::string> &args)
{
    if (args.size() == 2)
    {  
        std::string c_name  = args.front();
        std::string message = args.back();
        Client      &client = _clients.at(usr_id);

        if (_channels.find(c_name) != _channels.end())
        {
            if (_channels.at(c_name).is_client(usr_id))
            {
                _channels.at(c_name).remove_client(usr_id);
                client.unset_channel();
            }
            else
                add_reply(usr_id, _clients.at(usr_id).get_nick(), ERR_NOTONCHANNEL, MSG_NOTONCHANNEL); 
        }
        else
            add_reply(usr_id, c_name, ERR_NOSUCHCHANNEL, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() < 2)
        add_reply(usr_id, "PART", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::kick_cmd(int usr_id, std::vector<std::string> &args)
{
    if ( args.size() == 2 || args.size() == 3 )
    {
        std::string c_name   = args.front();
        std::string t_name   = args.at(1);
        std::string message  = "";

        if (args.size() == 3)
            message  = args.back();
        if (_channels.find(c_name) != _channels.end())
        {
            Channel &target_channel =  _channels.at(c_name);
            if (target_channel.is_nick_used(t_name))
            {
                if (is_operator(usr_id))
                {
                    int target_id = target_channel.get_client_id(t_name);
                    target_channel.remove_client(target_id);
                    _clients.at(target_id).unset_channel();
                    if (message != "")
                        add_reply(target_id, _clients.at(target_id).get_nick(), RPL_PRIVMSG, message);
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

void    Server::topic_cmd(int usr_id, std::vector<std::string> &args)
{
    if ( args.size() == 1 || args.size() == 2)
    {
        std::string c_name   = args.front();
        std::string topic    = args.back();
        if (_channels.find(c_name) != _channels.end())
        {
            if (args.size() == 1)
            {
                if (_channels.at(c_name).get_topic() != "")
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
    else if (args.size() < 2)
        add_reply(usr_id, "TOPIC", ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS);
}

void    Server::join_cmd(int usr_id, std::vector<std::string> &args)
{
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

std::vector<std::string>    split_command(std::string message)
{
    size_t                      sep;
    size_t                      i;
    std::vector<std::string>    tokens;
    std::string                 token;

    sep = 0;
    for (i = 0; sep != std::string::npos; i = sep + 1)
    {
        sep = message.find_first_of(" \0", i);
        if ((token = message.substr(i, sep - i)).size() != 0)
            tokens.push_back(token);
    }
    return tokens;
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
        std::vector<std::string> command_list = split_command(command);
        if (command_list.size() > 0)
        {
            std::string command_name = command_list.front();
            if ( _commands.find(command_name) != _commands.end())
            {
                command_list.erase(command_list.begin());
                (this->*_commands[command_name])(fd, command_list);
            }
        }
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