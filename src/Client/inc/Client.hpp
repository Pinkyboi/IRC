#ifndef CLIENT_H
# define CLIENT_H

# include <queue>
# include <string>
# include <netinet/in.h>
# include <netdb.h>
# include <list>
# include <map>
# include <algorithm>
# include "CommandBuffer.hpp"

# define MAX_COMMAND_SIZE 512

# define MODE_I (uint16_t)((uint16_t)(0x1) << 3)
# define MODE_W (uint16_t)((uint16_t)(0x1) << 2)

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
        void        set_mode(const std::string &mode);
        void        set_invisible(void);
        void        unset_invisible(void);
        void        set_wallop(void);
        void        unset_wallop(void);
        void        update_registration();
        void        add_channel(const std::string &channel_name);
        void        remove_channel(const std::string &channel_name);
    public:
        void                        get_addr() const;
        int                         get_id() const;
        int                         get_status() const;
        bool                        get_pass_validity() const;
        std::list <std::string>     get_channels() const;
        std::string                 get_nick() const;
        std::string                 get_username() const;
        std::string                 get_real_name() const;
        std::string                 get_command();
        std::string                 get_serv_id() const;
        std::string                 get_modes() const;
        bool                        is_in_channel(std::string &c_name) const;
        bool                        is_visible() const;
    public:
        bool                        handle_modes(std::string mode);

    private:
        typedef void (Client::*ModeFunc)(void);
        int                                                 _id;
        bool                                                _pass_validity;
        char                                                _addr[NI_MAXHOST];
        std::string                                         _nick;
        std::string                                         _username;
        std::string                                         _real_name;
        std::list<std::string>                              _channels;
        std::queue< std::pair< bool, CircularBuffer *> >    _commands;
        std::map<char, ModeFunc>                            _set_modes;
        std::map<char, ModeFunc>                            _unset_modes;
        int                                                 _status;
        bool                                                _visible;
        uint16_t                                            _modes;

};

#endif
