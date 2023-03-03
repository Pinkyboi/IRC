#ifndef CLIENT_H
# define CLIENT_H

# include <netinet/in.h>

class Client
{
    public:
        Client(int fd, struct sockaddr addr);
        ~Client();

    private:
        int             _fd;
        struct in_addr  _addr;
};

#endif