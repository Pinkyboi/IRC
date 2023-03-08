#ifndef SERVER_H
# define SERVER_H

# include "Client.hpp"
# include "Channel.hpp"

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
# include <list>

# define CONN_LIMIT 256
# define MAX_COMMAND_SIZE 512

class Server
{
    typedef void (Server::*cmd_func)(int, std::vector<std::string> &);

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
        void                            setup();
        void                            start();
    private:
                                        Server(const char *port, const char *pass);
        void                            print_msg(int fd);
        void                            send_msg(int fd, const std::string &msg);
        void                            accept_connection();
        void                            remove_connection(int user_id);
        void                            init_commands();
    private:
        void                            list_cmd(int usr_id, std::vector<std::string> &args);
        void                            nick_cmd(int usr_id, std::vector<std::string> &args); // basic version done
        void                            user_cmd(int usr_id, std::vector<std::string> &args); // basic version done
        void                            pass_cmd(int usr_id, std::vector<std::string> &args);
        void                            kick_cmd(int usr_id, std::vector<std::string> &args); // basic version done
        void                            join_cmd(int usr_id, std::vector<std::string> &args); // basic version done
        void                            part_cmd(int usr_id, std::vector<std::string> &args); // basic version done
        void                            msg_cmd(int  usr_id, std::vector<std::string> &args);
    private:
        static Server                           *_instance;
    private:
        const char                              *_port;
        const char                              *_pass;
        int                                     _sockfd;
        struct pollfd                           _pfds[CONN_LIMIT];
        int                                     _nfds;
        std::map<int, Client>                   _clients;
        std::map<const std::string, Channel>    _channels;
        std::map<std::string, cmd_func>         _commands;
};

#endif