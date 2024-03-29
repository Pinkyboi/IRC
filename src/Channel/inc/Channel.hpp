#ifndef CHANNEL_H
#define CHANNEL_H

#include <map>
#include <list>
#include <string>
#include <algorithm>
#include <sstream>
#include "Client.hpp"

#define MODE_T (uint16_t)(0x1) //Topic lock, which means that only channel operators can change the channel topic.
#define MODE_N (uint16_t)((uint16_t)(0x1) << 1) //The channel does not allow messages from users who are not in the channel.
#define MODE_S (uint16_t)((uint16_t)(0x1) << 2) //The channel is secret, which means it will not be listed on channel lists.
#define MODE_I (uint16_t)((uint16_t)(0x1) << 3) //The channel is invite-only, which means that only users who have been invited by the channel operators can join the channel.
#define MODE_M (uint16_t)((uint16_t)(0x1) << 4) //The channel is moderated, which means that only users who have been granted voice or operator status can send messages.
#define MODE_L (uint16_t)((uint16_t)(0x1) << 7) //The channel has a limit on the number of users who can join the channel.
#define MODE_B (uint16_t)((uint16_t)(0x1) << 8) //Bans a user from the channel.
#define MODE_K (uint16_t)((uint16_t)(0x1) << 6) //The channel has a password, which must be entered to join the channel.
#define MODE_O (uint16_t)((uint16_t)(0x1) << 9) //Grants operator status to a user.
#define MODE_V (uint16_t)((uint16_t)(0x1) << 10) //Grants voice status to a user.

class Channel
{
    public:
                                Channel(Client &client ,const std::string name);
                                ~Channel();
        std::string             get_member_prefix(Client &client);
        void                    add_client(Client& client);
        void                    remove_client(int client_id);
        void                    part_client(int client_id);
    public:
        bool                    is_client(int client_id);
        static std::string      get_valid_channel_name(std::string name);
    public:
        std::map<int, Client&>  &get_present_clients(void);
        std::map<int, Client&>  &get_clients(void);
        Client&                 get_client(int client_id);
        std::string             get_name() const;
        std::string             get_topic() const;
        std::string             get_name_with_topic() const;
        std::string             get_key() const;
        std::string             get_owner_nick() const;
        std::string             get_modes() const;
        std::string             get_mode_args() const;
        std::string             get_modes_with_args() const;
        size_t                 get_clients_count() const;
        size_t                 get_present_count() const;
        bool                    parse_mode(std::string mode, uint32_t target = 0);
    public:
        void                    set_topic(std::string topic);
    public:
        void                    set_mode(std::string mode, void *mode_argument = NULL);
        void                    unset_mode(std::string mode, void *mode_argument = NULL);
    public:
        bool                    is_topic_lock() const;
        bool                    is_there_space() const;
        bool                    is_channel_client_only() const;
        bool                    is_channel_secret() const;
        bool                    is_channel_moderated() const;
        bool                    is_channel_invite_only() const;
        bool                    is_channel_protected() const;
        bool                    is_key_valid(const std::string &key) const;
    public:
        bool                    is_client_unmute(Client &client) const;
        bool                    is_client_banned(Client &client) const;
        bool                    is_client_operator(Client &client) const;
        bool                    is_client_invited(Client &client) const;
        bool                    is_client_present(Client &client) const;
        bool                    is_client_owner(Client &client) const;
        void                    add_to_invites(Client &client);
        void                    remove_from_invites(int client_id);
    private:
        void                    add_operator(Client& client);
    private:
        bool                    set_mode_t    (std::queue<std::string> &mode_argument);
        bool                    set_mode_n    (std::queue<std::string> &mode_argument);
        bool                    set_mode_s    (std::queue<std::string> &mode_argument);
        bool                    set_mode_m    (std::queue<std::string> &mode_argument);
        bool                    set_mode_k    (std::queue<std::string> &mode_argument);
        bool                    set_mode_i    (std::queue<std::string> &mode_argument);
        bool                    set_mode_l    (std::queue<std::string> &mode_argument);
        bool                    unset_mode_t  (std::queue<std::string> &mode_argument);
        bool                    unset_mode_n  (std::queue<std::string> &mode_argument);
        bool                    unset_mode_s  (std::queue<std::string> &mode_argument);
        bool                    unset_mode_m  (std::queue<std::string> &mode_argument);
        bool                    unset_mode_k  (std::queue<std::string> &mode_argument);
        bool                    unset_mode_i  (std::queue<std::string> &mode_argument);
        bool                    unset_mode_l  (std::queue<std::string> &mode_argument);
    private:
        bool                    set_mode_v    (std::queue<std::string> &mode_argument);
        bool                    set_mode_b    (std::queue<std::string> &mode_argument);
        bool                    set_mode_o    (std::queue<std::string> &mode_argument);
        bool                    unset_mode_v  (std::queue<std::string> &mode_argument);
        bool                    unset_mode_b  (std::queue<std::string> &mode_argument);
        bool                    unset_mode_o  (std::queue<std::string> &mode_argument);
    public:
        std::string             handle_modes(std::string mode, std::queue<std::string>& mode_arg);
    public:
    private:
        typedef bool (Channel::*ModeFunc)(std::queue<std::string>&);
        std::map<char, ModeFunc>    _set_modes;
        std::map<char, ModeFunc>    _unset_modes;
        const std::string           _name;
        std::string                 _topic;
        Client&                     _owner;
        uint16_t                    _modes;
        std::string                 _key;
        unsigned long               _limit;
        std::map<int, Client&>      _clients;
        std::map<int, Client&>      _operators;
        std::map<int, Client&>      _voices;
        std::map<int, Client&>      _present;
        std::list<std::string>      _bans;
        std::map<int, Client&>      _invites;
};

#endif
