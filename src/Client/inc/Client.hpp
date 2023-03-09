#ifndef CLIENT_H
# define CLIENT_H

# include <queue>
# include <string>
# include <netinet/in.h>
# include "CommandBuffer.hpp"

# define MAX_COMMAND_SIZE 512
class Client
{
    public:
        Client(int id, struct sockaddr addr);
        ~Client();
    public:
        void        add_command(std::string cmd);
    public:
        void        set_active_nick(const std::string &nick);
        void        set_nick(const std::string &nick);
        void        set_username(const std::string &username);
        void        set_channel(const std::string &channel);
        void        unset_channel();
    public:
        void        get_addr() const;
        int         get_id() const;
        std::string get_nick() const;
        std::string get_active_nick() const;
        std::string get_username() const;
        std::string get_command();
        std::string get_channel() const;
    private:
        int                                                 _id;
        struct in_addr                                      _addr;
        std::string                                         _nick;
        std::string                                         _active_nick;
        std::string                                         _username;
        std::string                                         _active_channel;
        std::queue< std::pair< bool, CircularBuffer *> >    _commands;

};

#endif