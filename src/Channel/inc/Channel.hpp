#ifndef CHANNEL_H
#define CHANNEL_H

#include <map>
#include "Client.hpp"

class Channel
{
    public:
                                Channel(const std::string name);
                                ~Channel();
        void                    add_client(Client& client);
        void                    remove_client(int client_id);
    public:
        bool                    is_client(int client_id);
        bool                    is_nick_used(std::string& nick);
    public:
        int                     get_client_id(std::string& nick);
        Client&                 get_client(int client_id);
        std::string             get_name() const;
        std::string             get_topic() const;
        int                     get_clients_count() const;
    public:
        void                    set_topic(std::string topic);
        void                    update_nick(int client_id, std::string nick);
    private:
        const std::string           _name;
        std::string                 _topic;
        std::map<std::string, int>  _nicks;
        std::map<int, Client&>      _clients;
};

#endif
