#ifndef SERVER_H
# define SERVER_H

# include "Irc.h"
# include "Client.h"

# define CONN_LIMIT 256

class Server
{
    public:
        Server(const char *port, const char *pass);
        ~Server();

        // Getters
        struct sockaddr_in  *get_sockaddr();
        int                 get_sockfd();
        const char          *get_port();
        void                set_sockaddr(struct sockaddr_in *addr);

        void                start();
        void                setup();
        void                loop();
        void                accept_connection();


    private:
        const char                      *_port;
        const char                      *_pass;
        int                             _sockfd;
        struct sockaddr_in              *_sockaddr;
        struct pollfd                   _pfds[CONN_LIMIT];
        int                             _nfds;
        std::map<int, Client>           _clients;
};

#endif