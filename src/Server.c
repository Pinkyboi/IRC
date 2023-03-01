#include "Server.h"

Server::~Server()
{}

Server::Server(const char *port, const char *pass): _port(port), _pass(pass)
{
    this->_conn_limit = 10;
    this->_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    fcntl(this->_sockfd, F_SETFL, O_NONBLOCK);
    if (this->_sockfd < 0)
        exit(-1);
    std::cout << this->_sockfd << std::endl;
}

int Server::get_conn_limit()
{
    return this->_conn_limit;
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

int    main()
{
    struct sockaddr addr;
    socklen_t       addrlen;
    Server  serv("6667", "mokzwina");
    struct addrinfo hints;
    struct addrinfo *res, *p;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    std::cout << getaddrinfo("127.0.0.1", serv.get_port(), &hints, &res) << std::endl;
        
    serv.set_sockaddr((struct sockaddr_in *)(res->ai_addr));
    printf("%s\n", inet_ntoa(serv.get_sockaddr()->sin_addr));
    // if (connect(serv.get_sockfd(), (struct sockaddr *)serv.get_sockaddr(), sizeof(struct sockaddr_in)) < 0)
    // {
    //     // std::cout << "mok 1\n";
    
    //     perror("connect");
    //     exit(errno);
    // }
    if (bind(serv.get_sockfd(), (struct sockaddr *)serv.get_sockaddr(), sizeof(struct sockaddr_in)) < 0)
    {

        perror("bind");
        exit(errno);
    }

        std::cout << "mok 2\n";


    if (listen(serv.get_sockfd(), serv.get_conn_limit()) < 0)
    {

        perror("listen");
        exit(errno);
    }
        std::cout << "mok 3\n";

    struct pollfd pfds[10];
    pfds[0].fd = serv.get_sockfd();
    std::cout << "pfds[0].fd: " << pfds[0].fd << std::endl;

    int i = 0;
    while (true)
    {
        std::cout << "poll count\n";
        int poll_count = poll(pfds, 10, -1);
        std::cout << "poll count: " << poll_count << "\n";
        if (poll_count == -1)
        {
            perror("poll");
            exit(-1);
        }
        for (int i = 0; i < 1; i++)
        {
            if (pfds[i].revents & POLLIN)
            {
                    printf("Errr\n");
                if (pfds[i].fd == serv.get_sockfd())
                {
                    int fd = accept(serv.get_sockfd(), (struct sockaddr *)&addr, &addrlen);

                    if (fd < 0)
                    {

                        perror("accept");
                        exit(errno);
                    }
                    else
                    {
                        char buf[256] = "hello world\0";
                        send(fd, buf, sizeof(buf), 0);
                        std::cout << "new connection: " << ++i << "\n";
                    }
                }
            }
        }
        // std::cout << "mok 4\n";
    }
    return (0);
}