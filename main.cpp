
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

int main()
{
    try {
        signal(SIGINT, sig_handler);
        Server::initServer("6667", "mokzwina");
        Server *serv =  Server::getInstance();

        serv->setup();
        serv->start();
    }
    catch (Server::ServerException & e) {
        std::cout << e.what() << std::endl;
    }
    return (0);
}