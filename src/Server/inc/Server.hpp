#ifndef SERVER_H
# define SERVER_H

# include "Client.hpp"
# include "Channel.hpp"
# include "Parser.hpp"
# include "IRCReplies.hpp"

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
# include <string>
# include <sstream>

# define CONN_LIMIT 256
# define MAX_COMMAND_SIZE 512

#define CRLN "\r\n"
class Server
{
    typedef void (Server::*cmd_func)(int);
    struct case_insensitive_cmp {
        bool operator() (const std::string& s1, const std::string& s2) const {
            std::string str1(s1.length(),' ');
            std::string str2(s2.length(),' ');
            std::transform(s1.begin(), s1.end(), str1.begin(), tolower);
            std::transform(s2.begin(), s2.end(), str2.begin(), tolower);
            return  str1 < str2;
        }
    };

    public:
        class ServerException : public std::exception
        {
            public:
                ServerException(const char *msg):_msg(msg){};
                virtual const char* what() const throw() { return this->_msg; };
            private:
                const char  *_msg;
        };
    public:
        static std::string _servername;
        static std::string _motd;
    public:
        static Server*                  getInstance();
        static void                     initServer(const char *port, const char *pass);
        static void                     deleteInstance();
    public:
        ~Server();
        void                            setup();
        void                            start();
    public:
        bool                            is_nick_used(std::string& nick);
        void                            add_client(int id, struct sockaddr addr);
        void                            add_nick(int usr_id, std::string nick);
        void                            update_nick(int client_id, std::string nick);
    private:
                                        Server(const char *port, const char *pass);
        void                            receive_request(int fd);
        void                            send_replies();
        void                            accept_connection();
        void                            remove_connection(int user_id);
        void                            init_commands();
        std::string                     who_information(Client &client, const std::string &channel_name);
        void                            add_info_reply(int usr_id, const std::string &sender, const std::string &target,
                                                const std::string &code, const std::string &extra = "");
        void                            add_reply(int usr_id, const std::string &sender, const std::string &target,
                                                const std::string &code, const std::string &specifier, const std::string &extra = "", bool is_msg = true);
        void                            handle_commands(int fd, std::string &command);
    private:
        void                            pass_cmd        (int usr_id); // basic version done
        void                            user_cmd        (int usr_id); // basic version done
        void                            nick_cmd        (int usr_id); // basic version done
        void                            join_cmd        (int usr_id); // basic version done
        void                            part_cmd        (int usr_id); // basic version done
        void                            topic_cmd       (int usr_id);
        void                            names_cmd       (int usr_id); // basic version done
        void                            list_cmd        (int usr_id);
        void                            kick_cmd        (int usr_id); // basic version done
        void                            privmsg_cmd     (int usr_id);
        void                            notice_cmd      (int usr_id);
        void                            quit_cmd        (int usr_id);
        void                            mode_cmd        (int usr_id);
        void                            invite_cmd      (int usr_id);
        void                            who_cmd         (int usr_id);
        void                            userhost_cmd    (int usr_id);
        void                            ping_cmd        (int usr_id);
    private:
        static Server                                               *_instance;
    private:
        const char                                                  *_port;
        const std::string                                           _pass;
        int                                                         _sockfd;
        struct pollfd                                               _pfds[CONN_LIMIT];
        size_t                                                      _nfds;
        std::map<int, Client>                                       _clients;
        std::map<const std::string, Channel, case_insensitive_cmp>  _channels;
        std::map<const std::string, int, case_insensitive_cmp>      _nicks;
        std::map<std::string, cmd_func>                             _commands;
        std::queue< std::pair<int, std::string> >                   _replies;
        Parser                                                      _parser;
};


#endif