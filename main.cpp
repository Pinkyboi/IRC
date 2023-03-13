
#include "Server.hpp"
#include <csignal>

void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        std::cout << "Server shutting down..." << std::endl;
        Server::deleteInstance();
        exit(0);
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
        std::cout << "Usage: ./ircserv <port> <pass>" << std::endl;
    else
    {
        try {
            signal(SIGINT, sig_handler);

            Server::initServer(argv[1], argv[2]);
            Server *serv =  Server::getInstance();

            serv->setup();
            serv->start();
        }
        catch (Server::ServerException & e) {
            std::cout << "ircserv: " << e.what() << std::endl;
        }
    }
    return (0);
}