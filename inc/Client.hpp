#ifndef CLIENT_H
# define CLIENT_H

# include <queue>
# include <string.h>
# include <netinet/in.h>

class Client
{
    public:
        Client(int fd, struct sockaddr addr);
        ~Client();

    public:
        std::string get_command();
        void        add_command(const char *cmd, size_t size);

    private:
        int                                         _fd;
        struct in_addr                              _addr;
        // std::queue< std::pair<std::string, bool> >  _commands;
};

#endif