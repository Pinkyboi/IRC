#ifndef SERVER_H
# define SERVER_H

# include "Client.hpp"
// # include "Channel.hpp"

# include <sys/types.h>
# include <sys/socket.h>
# include <sys/errno.h>
# include <netinet/in.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <cstdlib>
# include <cstdio>
# include <cstring>
# include <unistd.h>
# include <stdexcept>
# include <iostream>
# include <fcntl.h>
# include <poll.h>
# include <map>



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
        static Server*                  getInstance();
        static void                     initServer(const char *port, const char *pass);
        static void                     deleteInstance();
    public:
        ~Server();
        void                            start();
        void                            setup();
    private:
                                        Server(const char *port, const char *pass);
        void                            print_msg(int fd);
        void                            accept_connection();
        void                            remove_connection(int user_index);
    private:
        static Server                   *_instance;
    private:
        const char                      *_port;
        const char                      *_pass;
        int                             _sockfd;
        struct pollfd                   _pfds[CONN_LIMIT];
        int                             _nfds;
        std::map<int, Client>           _clients;
        // std::map<const char*, Channel> _channels;
};

#endif