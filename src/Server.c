#include "Server.h"

Server::~Server()
{}

Server::Server(const char *port, const char *pass): _port(port)
{
    int on = 1;

    _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_sockfd > 0)
        throw Server::ServerException("socket() failed");

    setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    // setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    fcntl(_sockfd, F_SETFL, O_NONBLOCK);

    memset(_pfds, 0x00, sizeof(_pfds));
    _pfds[0].fd = _sockfd;
    _pfds[0].events = POLLIN;
    _nfds = 1;
}

const char * Server::get_port()
{
    return this->_port;
}

int Server::get_sockfd()
{
    return this->_sockfd;
}

struct sockaddr_in *Server::get_sockaddr()
{
    return this->_sockaddr;
}

void Server::set_sockaddr(struct sockaddr_in *addr)
{
    this->_sockaddr = addr;
}

void    Server::setup()
{
    struct addrinfo hints;
    struct addrinfo *res, *p;

    hints = (struct addrinfo){0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, this->get_port(), &hints, &res))
        throw Server::ServerException("Couldn't resolve host.");
    for (p = res; p; p = p->ai_next)
    {
        if (p->ai_family == AF_INET && p->ai_socktype == SOCK_STREAM)
            break;
    }
    if (p == NULL)
        throw Server::ServerException("Couldn't resolve host.");
    this->set_sockaddr((struct sockaddr_in *)(res->ai_addr));
    if (bind(this->get_sockfd(), (struct sockaddr *)this->get_sockaddr(), sizeof(struct sockaddr_in)) < 0)
        throw Server::ServerException("Couldn't bind socket.");
    if (listen(this->get_sockfd(), CONN_LIMIT) < 0)
        throw Server::ServerException("Couldn't listen on socket.");
}

void    Server::accept_connection()
{
    int             new_fd;
    struct sockaddr addr;
    socklen_t       addrlen;
    int             i;

    memset(&addr, 0x0, sizeof(struct sockaddr));
    addrlen = sizeof(struct sockaddr);
    if ((new_fd = accept(_sockfd, &addr, &addrlen)) > 0)
    {
        for (int i = 0; i < CONN_LIMIT; i++)
        {
            if (_pfds[i].fd  == 0)
            {
                std::cout << "new connection: " << new_fd << std::endl;
                _clients.insert(std::pair<int, Client>(new_fd, Client(new_fd, addr)));
                _pfds[i].fd = new_fd;
                _pfds[i].events = POLLIN | POLLHUP;
                _pfds[i].revents = 0;
                _nfds++;
                break ;
            }
        }
    }
}

void    Server::start()
{
    while (true)
    {
        if (poll(_pfds, _nfds, -1) > 0)
        {
            // Check for incoming connections.
            // Check for established connections messages.
            for (int i = 0; i < CONN_LIMIT; i++)
            {
                if (_pfds[i].fd == _sockfd && _pfds[i].revents & POLLIN)
                    accept_connection();
                else if (_pfds[i].revents & POLLHUP)
                {
                    memset(&_pfds[i], 0x00, sizeof(_pfds[i]));
                    _nfds--;
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