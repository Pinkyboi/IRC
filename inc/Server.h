#ifndef SERVER_H
# define SERVER_H
// # include <cstdint>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/errno.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <cstdlib>
# include <iostream>
# include <map>
# include <cstdio>
# include <fcntl.h>
# include <poll.h>
# include <vector>
# include <stdexcept>

# define CONN_LIMIT 256

class Server
{
    public:
        Server(const char *port, const char *pass);
        ~Server();
        void        start();
        void        setup();
        struct sockaddr_in *get_sockaddr();
        int         get_sockfd();
        const char  *get_port();
        void        loop();
        void        set_sockaddr(struct sockaddr_in *addr);

    private:
        const char          *_port;
        const char          *_pass;
        int                 _sockfd;
        struct sockaddr_in  *_sockaddr;
        struct pollfd       _pfds[CONN_LIMIT];
        int                 _nfds;
};

#endif