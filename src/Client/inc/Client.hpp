#ifndef CLIENT_H
# define CLIENT_H

# include <queue>
# include <string>
# include <netinet/in.h>
# include <netdb.h>
# include <list>
# include <algorithm>
# include "CommandBuffer.hpp"

# define MAX_COMMAND_SIZE 512
class Client
{
    public:
        typedef enum e_status
        {
            DOWN = 0,
            UNREGISTERED = 1,
            REGISTERED = 2,
        } t_status;
        Client(int id, struct sockaddr addr);
        ~Client();
    public:
        static bool    is_nick_valid(const std::string &nick);
    public:
        void        add_command(std::string cmd);
        bool        is_registered() const;
    public:
        void        set_nick(const std::string &nick);
        void        set_username(const std::string &username);
        void        set_real_name(const std::string &real_name);
        void        set_pass_validity(const bool validity);
        void        set_status(int status);
        void        update_registration();
        void        add_channel(const std::string &channel_name);
        void        remove_channel(const std::string &channel_name);
    public:
        void                        get_addr() const;
        int                         get_id() const;
        int                         get_status() const;
        bool                        get_pass_validity() const;
        bool                        is_in_channel(std::string &c_name) const;
        std::list <std::string>&    get_channels();
        std::string                 get_nick() const;
        std::string                 get_username() const;
        std::string                 get_real_name() const;
        std::string                 get_command();
        std::string                 get_serv_id() const;

    private:
        int                                                 _id;
        bool                                                _pass_validity;
        char                                                _addr[NI_MAXHOST];
        std::string                                         _nick;
        std::string                                         _username;
        std::string                                         _real_name;
        std::list<std::string>                              _channels;
        std::queue< std::pair< bool, CircularBuffer *> >    _commands;
        int                                                 _status;

};

#endif
