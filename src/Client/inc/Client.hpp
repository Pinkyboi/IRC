#ifndef CLIENT_H
# define CLIENT_H

# include <queue>
# include <string>
# include <netinet/in.h>
# include <CommandBuffer.hpp>

# define MAX_COMMAND_SIZE 512

class Client
{
    public:
        Client(int id, struct sockaddr addr);
        Client(){};
        ~Client();

    public:
        std::string get_command();
        void        add_command(const std::string &cmd);
    public:
        void        set_nick(const std::string &nick);
        void        set_username(const std::string &username);
    public:
        void        get_addr() const;
        int         get_id() const;
        std::string get_nick() const;
        std::string get_username() const;
    private:
        int                                         _id;
        struct in_addr                              _addr;
        std::string                                 _nick;
        std::string                                 _username;
        std::queue< std::pair< CircularBuffer, bool> >  _commands;
};

#endif
