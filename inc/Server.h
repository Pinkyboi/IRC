#ifndef SERVER_H
# define SERVER_H

# include "Irc.h"
# include "Client.h"

# define CONN_LIMIT 256

class Server
{
    public:
        class ServerException : public std::exception
        {
            public:
                ServerException(const char *msg):_msg(msg){};
                const char* what() { return this->_msg; };
            private:
                const char  *_msg;
        };

    public:
        Server(const char *port, const char *pass);
        ~Server();

        void                start();
        void                setup();

    private:
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