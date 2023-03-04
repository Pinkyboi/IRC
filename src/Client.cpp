# include "Client.hpp"

Client::Client(int fd, struct sockaddr addr)
{
    _fd = fd;
    _addr = ((struct sockaddr_in *)&addr)->sin_addr;
}

Client::~Client()
{}

// std::string Client::get_command()
// {
//     if (_cmd_ready)
//         return _commands.pop();
// }

// void    Client::add_command(const char *cmd)
// {
//     // std::string command(cmd);
//     // std::string::iterator it = comment.find("\\r\\n");
//     // if (it != command.end())
//     //     command.erase(it, it + 4);
//     // _commands.push(command);
//     // _commands.push(std::string(cmd));
//     // _cmd_ready = true;
//     // std::string command_string(cmd);
//     // std::iterator it =  command_string.being();
//     // while(it != command_string.end())
//     // {
//     //     it = command_string.find("\\r\\n");

//     // }
    
// }