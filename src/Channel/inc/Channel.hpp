#ifndef CHANNEL_H
#define CHANNEL_H

#include <map>
#include "Client.hpp"

#define T_MODE 0x1 //Topic lock, which means that only channel operators can change the channel topic.
#define N_MODE 0x1 << 1 //The channel does not allow messages from users who are not in the channel.
#define S_MODE 0x1 << 2 //The channel is secret, which means it will not be listed on channel lists.
#define I_MODE 0x1 << 3 //The channel is invite-only, which means that only users who have been invited by the channel operators can join the channel.
#define M_MODE 0x1 << 4 //The channel is moderated, which means that only users who have been granted voice or operator status can send messages.
#define P_MODE 0x1 << 5 //The channel is private, which means that it will not be listed on channel lists.
#define K_MODE 0x1 << 6 //The channel has a password, which must be entered to join the channel.
#define L_MODE 0x1 << 7 //The channel has a limit on the number of users who can join the channel.
#define B_MODE 0x1 << 8 //Bans a user from the channel.
#define O_MODE 0x1 << 9 //Grants operator status to a user.
#define V_MODE 0x1 << 10 //Grants voice status to a user.

class Channel
{
    public:
                                Channel(const std::string name);
                                ~Channel();
        void                    add_client(Client& client);
        void                    remove_client(int client_id);
    public:
        bool                    is_client(int client_id);
    public:
        std::map<int, Client&>  &get_clients(void);
        Client&                 get_client(int client_id);
        std::string             get_name() const;
        std::string             get_topic() const;
        int                     get_clients_count() const;
    public:
        void                    set_topic(std::string topic);
    private:
        const std::string           _name;
        std::string                 _topic;
        std::map<int, Client&>      _clients;
        uint16_t                    _modes;
};

#endif
