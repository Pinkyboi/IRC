#ifndef CHANNEL_H
#define CHANNEL_H

#include <map>
#include "Client.hpp"

class Channel
{
    public:
                    Channel(const char * name);
                    ~Channel();
        void        addClient(Client& client);
        void        removeClient(Client& client);
        Client&     getClient(int fd);

    private:
        const char              * _name;
       // std::map<int, Client&>  _clients;
};

#endif
