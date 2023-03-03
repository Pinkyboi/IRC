# include "Client.hpp"

Client::Client(int fd, struct sockaddr addr)
{
    _fd = fd;
    _addr = ((struct sockaddr_in *)&addr)->sin_addr;
}

Client::~Client()
{}