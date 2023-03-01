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

class Server
{
    public:
        Server(const char *port, const char *pass);
        ~Server();
        struct sockaddr_in *get_sockaddr();
        int get_sockfd();
        const char * get_port();
        int get_conn_limit();
        void    loop();
        void set_sockaddr(struct sockaddr_in *addr);

    private:
        const char  *_port;
        const char  *_pass;
        int         _sockfd;
        // std::map<int, Client::Client> _users;
        int   _conn_limit;
        struct sockaddr_in  *_sockaddr;
};

#endif