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
};

#endif
