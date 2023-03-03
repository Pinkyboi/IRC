#include "Server.hpp"

Server::~Server(){}

Server::Server(const char *port, const char *pass): _port(port), _nfds(0)
{
    int on = 1;

    if ((_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        throw Server::ServerException("Couldn't create socket.");
    setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    fcntl(_sockfd, F_SETFL, O_NONBLOCK);
    memset(_pfds, 0x00, sizeof(_pfds));
    _pfds[_nfds]= (pollfd){ .fd = _sockfd,
                            .events = POLLIN };
    _nfds++;
}

void    Server::setup()
{
    struct addrinfo hints;
    struct addrinfo *res, *p;

    hints = (struct addrinfo){  .ai_family = AF_INET,
                                .ai_socktype = SOCK_STREAM,
                                .ai_flags = AI_PASSIVE };
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
        _pfds[_nfds] = (struct pollfd){ .fd = new_fd,
                                        .events = POLLIN };
        _nfds++;
    }
}

void    Server::remove_connection(int user_index)
{
    struct pollfd *userfd = &_pfds[user_index];

    std::cout << "client disconnected: " << userfd->fd << std::endl;
    close(userfd->fd);
    memmove(userfd, userfd + 1, sizeof(struct pollfd) * (_nfds - user_index - 1));
    _nfds--;
}

void    Server::print_msg(int fd)
{
    static char msg_buffer[512];

    memset(msg_buffer, 0x0, sizeof msg_buffer);
    if (recv(fd, msg_buffer, 512, MSG_DONTWAIT) > 0)
        std::cout << "msg: " << msg_buffer << std::endl;
}

void    Server::start()
{
    while (true)
    {
        if (poll(_pfds, _nfds, -1) > 0)
        {
            // Check for incoming connections.
            // Check for established connections messages.
            for (int i = 0; i < _nfds; i++)
            {
                // if (_pfds[i].revents & (POLLHUP | POLLERR))
                //     remove_connection(i);
                if (_pfds[i].revents & POLLIN)
                {
                    if (_pfds[i].fd != _sockfd)
                        print_msg(_pfds[i].fd);
                    else
                        accept_connection();
                }
            }
        }
    }
}


int main()
{
    try {
        Server  serv("6667", "mokzwina");
        serv.setup();
        serv.start();
    }
    catch (Server::ServerException & e) {
        std::cout << e.what() << std::endl;
    }

    return (0);
}