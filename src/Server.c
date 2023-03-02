#include "Server.h"

Server::~Server()
{}

Server::Server(const char *port, const char *pass): _port(port), _pass(pass)
{
    int on = 1;

    _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_sockfd < 0)
        throw std::exception();

    setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    fcntl(_sockfd, F_SETFL, O_NONBLOCK);

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

    int rv;
    if ((rv = getaddrinfo(NULL, this->get_port(), &hints, &res)))
    {
        std::cout << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        throw std::exception();
    }
    for (p = res; p; p = p->ai_next)
    {
        if (p->ai_family == AF_INET && p->ai_socktype == SOCK_STREAM)
            break;
    }

    if (p == NULL)
    {
        throw std::exception();
    }

    this->set_sockaddr((struct sockaddr_in *)(res->ai_addr));

    if (bind(this->get_sockfd(), (struct sockaddr *)this->get_sockaddr(), sizeof(struct sockaddr_in)) < 0)
        throw std::exception();
    if (listen(this->get_sockfd(), CONN_LIMIT) < 0)
        throw std::exception();
}

void    Server::start()
{
    struct sockaddr addr;
    socklen_t       addrlen;

    int j = 1;
    while (true)
    {
        int poll_count = poll(_pfds, _nfds, -1);
        if (poll_count == -1)
        {
            perror("poll");
            exit(-1);
        }
        for (int i = 0; i < 2; i++)
        {
            if (_pfds[i].revents & POLLIN)
            {
                if (_pfds[i].fd == get_sockfd())
                {
                    int fd = accept(get_sockfd(), &addr, &addrlen);

                    if (fd < 0)
                    {
                        perror("accept");
                        exit(errno);
                    }
                    else
                    {
                        std::cout << "new connection from: " << inet_ntoa(((struct sockaddr_in *)&addr)->sin_addr) << std::endl;
                        char buf[256] = "hello world\n";
                        send(fd, buf, sizeof(buf), 0);
                        _pfds[j].fd = fd;
                        _pfds[j].events = POLLIN | POLLHUP;
                    }
                }
            }
            else if (_pfds[j].revents & POLLHUP)
            {
                _pfds[j].fd = 0;
                _pfds[j].events = 0;
                _pfds[j].revents = 0;
                printf("mok zwina\n");
            }
            _pfds[i].revents = 0;
        }
        std::cout << "mok 4\n";
    }
}


int main()
{
    try {
        Server  serv("6667", "mokzwina");
        serv.setup();
        serv.start();
    } catch (std::exception & e) {
        std::cout << e.what() << std::endl;
    }

    return (0);
}