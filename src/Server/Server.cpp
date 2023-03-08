#include "Server.hpp"

Server *Server::_instance = NULL;

Server::~Server()
{
    for (int i = 0; i < _nfds; i++)
        close(_pfds[i].fd);
}

Server::Server(const char *port, const char *pass): _port(port), _nfds(0)
{
    bool on = true;

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
        _clients.insert(std::pair<int, Client>(new_fd, Client(new_fd, addr)));
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
    _nfds--;
}

void    Server::send_msg(int fd, const std::string &msg)
{
    send(fd, msg.c_str(), msg.length(), 0);
}

void    list_cmd(int usr_id, std::string &c_name)
{

}

void    Server::nick_cmd(int usr_id, std::vector<std::string> &args)
{
    std::string nick  = args.front();

    if (_clients.at(usr_id).get_channel() == "")
        _clients.at(usr_id).set_nick(nick);
    else
    {
        Channel &current_channel = _channels.at(_clients.at(usr_id).get_channel());
        if (!current_channel.is_nick_used(nick))
            current_channel.set_nick(usr_id, nick);
    }
}

void    Server::part_cmd(int usr_id, std::vector<std::string> &args)
{
    std::string c_name  = args.front();
    std::string message = args.back();
    Client      &client = _clients.at(usr_id);

    if (_channels.find(c_name) != _channels.end())
    {
        if (_channels.at(c_name).is_client(usr_id))
            _channels.at(c_name).remove_client(usr_id);
    }
    client.unset_channel();
}

void    Server::kick_cmd(int usr_id, std::vector<std::string> &args)
{
    std::string c_name  = args.front();
    std::string message = args.back();
    std::string t_name  = args.at(1);

    Channel    &channel   = _channels.at(c_name);    
    int        target_id = channel.get_client_id(t_name);
    if (_channels.find(c_name) != _channels.end())
    {
        if (_channels.at(c_name).is_operator(usr_id) && _channels.at(c_name).is_client(target_id))
            _channels.at(c_name).remove_client(target_id);
    }
}

void    Server::join_cmd(int usr_id, std::vector<std::string> &args)
{
    std::string c_name  = args.front();
    std::string message = args.back();
    Client      &client = _clients.at(usr_id);

    if (_channels.find(c_name) == _channels.end())
        _channels.insert(std::pair<std::string, Channel>(c_name, Channel(client, c_name)));
    else
        _channels.at(c_name).add_client(client);
    client.set_channel(c_name);
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
        std::cout << command << std::endl;
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