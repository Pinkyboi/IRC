#ifndef CLIENT_H
# define CLIENT_H

# include <queue>
# include <string>
# include <netinet/in.h>
# include <netdb.h>
# include "CommandBuffer.hpp"

# define MAX_COMMAND_SIZE 512
class Client
{
    public:
        Client(int id, struct sockaddr addr);
        ~Client();
    public:
        void        add_command(std::string cmd);
        bool        is_registered() const;
    public:
        void        set_nick(const std::string &nick);
        void        set_username(const std::string &username);
        void        set_real_name(const std::string &real_name);
        void        set_channel(const std::string &channel);
        void        set_pass_validity(const bool validity);
        void        set_status(int status);
        void        unset_channel();
    public:
        void        get_addr() const;
        int         get_id() const;
        int         get_status() const;
        std::string get_nick() const;
        std::string get_username() const;
        std::string get_real_name() const;
        std::string get_command();
        std::string get_channel() const;
        std::string get_serv_id() const;

    private:
        typedef enum e_status
        {
            DOWN = 0,
            UNREGISTERED = 1,
            REGISTERED = 2,
        } t_status;
        int                                                 _id;
        bool                                                _pass_validity;
        char                                                _addr[NI_MAXHOST];
        std::string                                         _nick;
        std::string                                         _username;
        std::string                                         _real_name;
        std::string                                         _active_channel;
        std::queue< std::pair< bool, CircularBuffer *> >    _commands;
        int                                                 _status;

};

#endif
