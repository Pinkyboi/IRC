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

void    Server::print_msg(int fd)
{
    static char msg_buffer[MAX_COMMAND_SIZE];
    size_t      msg_len;
    char * command;

    if ( (msg_len = recv(fd, msg_buffer, MAX_COMMAND_SIZE, MSG_DONTWAIT)) > 0)
    {
        msg_buffer[msg_len] = '\0';
        _clients[fd].add_command(std::string(msg_buffer));
    }
    while((command = _clients[fd].get_command()) != NULL)
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