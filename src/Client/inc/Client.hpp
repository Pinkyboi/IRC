#ifndef CLIENT_H
# define CLIENT_H

# include <queue>
# include <string>
# include <netinet/in.h>

class Client
{
    public:
        Client(int id, struct sockaddr addr);
        Client(){};
        ~Client();

    public:
        std::string get_command();
        void        add_command(const std::string &cmd);
        void        get_addr();
        int         get_id() const;
    private:
        int                                         _id;
        struct in_addr                              _addr;
        std::queue< std::pair<std::string, bool> >  _commands;
};

#endif
