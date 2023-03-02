# include "Client.h"

Client::Client(int fd, struct sockaddr addr)
{
    _fd = fd;
    _addr = ((struct sockaddr_in *)&addr)->sin_addr;
    std::cout << "Constructin client with fd:" << fd << " " << inet_ntoa(_addr) << std::endl;
}

Client::~Client()
{}