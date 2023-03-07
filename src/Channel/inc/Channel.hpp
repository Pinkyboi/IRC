#ifndef CHANNEL_H
#define CHANNEL_H

#include <map>
#include "Client.hpp"

class Channel
{
    public:
                                Channel(Client& creator, const char * name);
                                ~Channel();
        void                    add_client(Client& client);
        Client&                 get_client(int fd);
        void                    remove_client(Client& client);
        void                    add_operator(Client& client);

    private:
        const char              *_name;
        char                    *_topic;
        uint16_t                _modes;
        std::map<int, Client&>  _clients;
        std::map<int, Client&>  _operators;
};

#endif
