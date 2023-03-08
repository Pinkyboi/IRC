#ifndef CHANNEL_H
#define CHANNEL_H

#include <map>
#include "Client.hpp"

class Channel
{
    public:
                                Channel(Client& creator, const std::string name);
                                ~Channel();
        void                    add_client(Client& client);
        void                    add_operator(Client& client);
        void                    remove_client(int client_id);
    public:
        bool                    is_operator(int client_id);
        bool                    is_client(int client_id);
    public:
        Client&                 get_client(int client_id);
        std::string             get_name() const;
        std::string             get_topic() const;
        uint16_t                get_modes() const;
    public:
        void                    set_topic(std::string topic);
        void                    set_modes(uint16_t modes); // to discuss
    private:
        const std::string       _name;
        std::string             _topic;
        uint16_t                _modes;  // in discussion
        std::map<int, Client&>  _clients;
        std::map<int, Client&>  _operators;
};

#endif
