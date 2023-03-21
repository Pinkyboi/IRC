#include "Server.hpp"

Server *Server::_instance = NULL;
std::string Server::_servername = "irc.3assifa.net";
std::string Server::_motd = "This is the message of the day";

Server::~Server()
{
    for (int i = 0; i < _nfds; i++)
        close(_pfds[i].fd);
}

Server::Server(const char *port, const char *pass): _port(port), _nfds(0), _pass(std::string(pass))
{
    bool on = true;

    init_commands();
    if ((_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        throw Server::ServerException("Couldn't create socket.");
    setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    fcntl(_sockfd, F_SETFL, O_NONBLOCK);
    memset(_pfds, 0x00, sizeof(_pfds));
    _pfds[_nfds]= (struct pollfd){  .fd = _sockfd,
                                    .events = POLLIN };
    _nfds++;
}

void    Server::init_commands()
{
    _commands.insert(std::pair<std::string, cmd_func>("PASS", &Server::pass_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("NICK", &Server::nick_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("USER", &Server::user_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("JOIN", &Server::join_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("PART", &Server::part_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("NAMES", &Server::names_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("KICK", &Server::kick_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("TOPIC", &Server::topic_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("LIST", &Server::list_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("PRIVMSG", &Server::privmsg_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("NOTICE", &Server::notice_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("QUIT", &Server::quit_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("MODE", &Server::mode_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("INVITE", &Server::invite_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("PING", &Server::ping_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("WHO", &Server::who_cmd));
}

bool    Server::is_nick_used(std::string& nick)
{
    return (_nicks.find(nick) != _nicks.end());
}

void    Server::setup()
{
    struct addrinfo hints;
    struct addrinfo *res, *p;

    hints = (struct addrinfo){  .ai_flags = AI_PASSIVE,
                                .ai_family = AF_INET,
                                .ai_socktype = SOCK_STREAM };
    if (getaddrinfo(NULL, this->_port, &hints, &res))
        throw Server::ServerException("Couldn't resolve host.");
    for (p = res; p; p = p->ai_next)
    {
        if (p->ai_family == AF_INET && p->ai_socktype == SOCK_STREAM)
            break;
    }
    if (p == NULL)
        throw Server::ServerException("Couldn't resolve host.");
    if (bind(this->_sockfd, p->ai_addr, sizeof(struct sockaddr_in)) < 0)
        throw Server::ServerException("Couldn't bind socket.");
    if (listen(this->_sockfd, CONN_LIMIT) < 0)
        throw Server::ServerException("Couldn't listen on socket.");
    freeaddrinfo(res);
}

void    Server::add_client(int id, struct sockaddr addr)
{
    _clients.insert(std::pair<int, Client>(id, Client(id, addr)));
}

void    Server::accept_connection()
{
    struct sockaddr addr;
    socklen_t       addrlen;
    int             new_fd;

    addr = (sockaddr){0};
    addrlen = sizeof(struct sockaddr_in);
    if ((new_fd = accept(_sockfd, &addr, &addrlen)) > 0)
    {
        std::cout << "new connection: " << new_fd << std::endl;
        add_client(new_fd, addr);
#if defined(__linux__)
        _pfds[_nfds] = (struct pollfd){ .fd = new_fd,
                                        .events = POLLIN | POLLRDHUP };
#elif defined(__APPLE__)
        _pfds[_nfds] = (struct pollfd){ .fd = new_fd,
                                        .events = POLLIN | POLLHUP };
#endif
        _nfds++;
    }
}

void    Server::remove_connection(int user_id)
{
    size_t i = 0;
    while (i < _nfds && _pfds[i].fd != user_id)
        i++;
    struct pollfd *userfd = &_pfds[i];
    Client& client = _clients.at(user_id);
    std::list<std::string> channels = client.get_channels();
    for (std::list<std::string>::iterator it = channels.begin(); it != channels.end(); it++)
        _channels.at(*it).remove_client(user_id);
    _nicks.erase(client.get_nick());
    _clients.erase(user_id);
    memmove(userfd, userfd + 1, sizeof(struct pollfd) * (_nfds - i));
    close(user_id);
    _nfds--;
}

void    Server::quit_cmd(int usr_id)
{
    std::string message = _parser.get_message();

    add_info_reply(usr_id, _servername, "QUIT", "", message);
    _clients.at(usr_id).set_status(Client::DOWN);
}

void    Server::privmsg_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    std::string message = _parser.get_message();
    Client& client = _clients.at(usr_id);
    std::string s_name = client.get_serv_id();
    if (message.size() == 0)
        add_info_reply(usr_id, _servername, ERR_NOTEXTTOSEND, s_name, MSG_NOTEXTTOSEND);
    if (args.size() == 1)
    {
        std::string t_name = args.front();
        if (_channels.find(t_name) != _channels.end())
        {
            Channel &t_channel = _channels.at(t_name);

            if (t_channel.is_channel_client_only() && t_channel.is_client(usr_id) == false)
                add_reply(usr_id, _servername, ERR_CANNOTSENDTOCHAN, client.get_nick(), t_name, MSG_CANNOTSENDTOCHAN);
            else if (t_channel.is_client_unmute(client) == false)
                add_reply(usr_id, _servername, ERR_CANNOTSENDTOCHAN, client.get_nick(), t_name, MSG_CANNOTSENDTOCHAN);
            else if (!t_channel.is_client_banned(_clients.at(usr_id)))
            {
                std::map<int, Client&> &clients = t_channel.get_present_clients();
                for (std::map<int, Client&>::iterator it = clients.begin(); it != clients.end(); it++)
                    if (it->first != usr_id)
                        add_info_reply(it->first, s_name, RPL_PRIVMSG, it->second.get_nick(), message);
            }
            else
                add_reply(usr_id, _servername, ERR_BANNEDFROMCHAN, client.get_nick(), t_name, MSG_BANNEDFROMCHAN);
        }
        else if (is_nick_used(t_name))
            add_info_reply(_nicks.at(t_name), s_name, RPL_PRIVMSG, t_name, message);
        else
            add_reply(usr_id, _servername, ERR_NOSUCHNICK, client.get_nick(), t_name, MSG_NOSUCHNICK);
    }
    else if (args.size() > 1)
        add_reply(usr_id, _servername, ERR_TOOMANYTARGETS, client.get_nick(), "PRIVMSG", MSG_TOOMANYTARGETS);
    else
        add_reply(usr_id, _servername, ERR_NORECIPIENT, client.get_nick(), "PRIVMSG", MSG_NORECIPIENT);
}


void    Server::mode_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    std::string message = _parser.get_message();
    size_t nargs = _parser.get_nargs();
    Client &client  = _clients.at(usr_id);

    if ( nargs >= 2 )
    {
        std::string t_name = args.front();
        std::string argument = "";
        std::string modes = args.at(1);
        if (nargs == 3)
            argument = args.back();
        if (modes.size() == 1)
            return ;
        if (_nicks.find(t_name) != _nicks.end())
        {
            if (client.get_nick() == t_name)
            {
                if (client.handle_modes(modes))
                    add_reply(usr_id, client.get_serv_id(), "MODE", t_name, modes);
            }
            else
                add_reply(usr_id, _servername, ERR_USERSDONTMATCH, client.get_nick(), t_name, MSG_USERSDONTMATCH);
        }
        else if (_channels.find(t_name) != _channels.end())
        {
            Channel& t_channel =  _channels.at(t_name);

            if (t_channel.is_client_operator(client))
            {
                std::queue <std::string> args_queue;
                if (args.size() > 2)
                {
                    for (std::vector<std::string>::iterator it = args.begin() + 2; it != args.end(); it++)
                        args_queue.push(*it);
                }
                std::string used_modes = t_channel.handle_modes(modes, args_queue);
                add_reply(usr_id, client.get_serv_id(), "MODE", t_name, used_modes);
            }
            else
                add_reply(usr_id, _servername, ERR_CHANOPRIVSNEEDED, client.get_nick(), t_name, MSG_CHANOPRIVSNEEDED);
        }
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, client.get_nick(), t_name, MSG_NOSUCHCHANNEL);
    }
    else if (nargs == 1)
    {
        std::string t_name = args.front();
        if (_nicks.find(t_name) != _nicks.end())
        {
            if (client.get_nick() == t_name)
                add_info_reply(usr_id, _servername, RPL_UMODEIS, t_name, client.get_modes());
            else
                add_reply(usr_id, _servername, ERR_USERSDONTMATCH, client.get_nick(), t_name, MSG_USERSDONTMATCH);
        }
        else if (_channels.find(t_name) != _channels.end())
            add_reply(usr_id, _servername, RPL_CHANNELMODEIS, client.get_nick(), t_name, _channels.at(t_name).get_modes_with_args());
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, client.get_nick(), t_name, MSG_NOSUCHCHANNEL);
    }
    else
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, client.get_nick(), "MODE", MSG_NEEDMOREPARAMS);
}

void    Server::notice_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    std::string message = _parser.get_message();
    std::string s_name = _clients.at(usr_id).get_serv_id();
    size_t nargs = _parser.get_nargs();

    if (nargs == 1)
    {
        std::string t_name = args.front();
        if (_channels.find(t_name) != _channels.end())
        {
            std::map<int, Client&> &clients = _channels.at(t_name).get_present_clients();
            for (std::map<int, Client&>::iterator it = clients.begin(); it != clients.end(); it++)
            {
                if (it->first != usr_id)
                    add_info_reply(it->first, s_name, RPL_PRIVMSG, it->second.get_nick(), message);
            }
        }
        else if (is_nick_used(t_name))
            add_info_reply(_nicks.at(t_name), s_name, RPL_PRIVMSG, t_name, message);
    }
}

std::string convert_to_string(int number)
{
    std::stringstream ss;

    ss << number;
    return ss.str();
}

void    Server::list_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();

    std::string nick = _clients.at(usr_id).get_nick();
    if (args.size() == 0)
    {
        add_reply(usr_id, _servername, RPL_LISTSTART, nick, "LIST", MSG_LISTSTART);
        for (std::map<const std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
        {
            if (it->second.is_channel_secret() == false || it->second.is_client(usr_id))
            {
                std::string msg = it->first + " " + convert_to_string(it->second.get_clients_count());
                add_reply(usr_id, _servername, RPL_LIST, nick, msg, it->second.get_topic());
            }
        }
        add_info_reply(usr_id, _servername, RPL_LISTEND, nick, MSG_LISTEND);
    }
    else if (args.size() == 1)
    {
        std::string c_name = args.at(0);
        if (_channels.find(c_name) != _channels.end())
        {
            Channel &channel = _channels.at(c_name);
            if (channel.is_channel_secret() == false || channel.is_client(usr_id))
            {
                std::string msg = c_name + " " + convert_to_string(channel.get_clients_count());
                add_reply(usr_id, _servername, RPL_LIST, nick, msg, channel.get_topic());
            }
        }
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, nick, c_name, MSG_NOSUCHCHANNEL);
        add_info_reply(usr_id, _servername, RPL_LISTEND, nick, MSG_LISTEND);
    }
}

void    Server::add_info_reply(int usr_id, const std::string &sender, const std::string &code,
                    const std::string &target, const std::string &extra)
{
    std::string replymsg = ":" + sender + " " + code + " " + target;
    if (extra.size())
    {
        replymsg += " ";
        replymsg += ":";
        replymsg += extra;
    }
    replymsg += CRLN;
    std::cout << "SENDED BY SERV: " << replymsg << std::endl;
    _replies.push(std::pair<int, std::string>(usr_id, replymsg));
}

void    Server::add_reply(int usr_id, const std::string &sender, const std::string &code,
                    const std::string &target, const std::string &specifier, const std::string &extra, bool is_msg)
{
    std::string replymsg = ":" + sender + " " + code + " " + target + " " + specifier;
    if (extra.size())
    {
        replymsg += " ";
        if (is_msg)
            replymsg += ":";
        replymsg += extra;
    }
    std::cout << "SENDED BY SERV: " << replymsg << std::endl;
    replymsg += CRLN;
    _replies.push(std::pair<int, std::string>(usr_id, replymsg));
}

void    Server::user_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    std::string real_name = _parser.get_message();
    Client &user = _clients.at(usr_id);

    if (args.size() < 3 || real_name.empty())
        add_info_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, user.get_nick(), MSG_NEEDMOREPARAMS);
    else if (user.is_registered())
        add_info_reply(usr_id, _servername, ERR_ALREADYREGISTRED, user.get_nick(), MSG_ALREADYREGISTRED);
    else
    {
        std::string username = args.front();
        std::string mode = args.at(1);
        user.set_username(username);
        user.set_real_name(real_name);
        user.set_mode(mode);
    }
}

void    Server::update_nick(int usr_id, std::string newnick)
{
    Client& client = _clients.at(usr_id);
    std::string oldnick = client.get_nick();
    if (Client::is_nick_valid(newnick) == false)
        add_reply(usr_id, _servername, ERR_ERRONEUSNICKNAME, client.get_nick(), newnick, MSG_ERRONEUSNICKNAME);
    else
    {
        add_info_reply(usr_id, client.get_serv_id(), "NICK", newnick);
        client.set_nick(newnick);
        _nicks.erase(oldnick);
        _nicks.insert(std::pair<std::string, int>(newnick, usr_id));
    }
}

void    Server::add_nick(int usr_id, std::string nick)
{
    _nicks.insert(std::pair<std::string, int>(nick, usr_id));
}

void    Server::nick_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    Client &client = _clients.at(usr_id);

    if ( args.size() == 1 )
    {
        std::string nick  = args.front();
        if ( is_nick_used(nick) ==  false )
        {
            if (client.is_registered())
                update_nick(usr_id, nick);
            else
            {
                if (Client::is_nick_valid(nick) == false)
                    add_reply(usr_id, _servername, ERR_ERRONEUSNICKNAME, client.get_nick(), nick, MSG_ERRONEUSNICKNAME);
                else
                {
                    client.set_nick(nick);
                    add_nick(usr_id, nick);
                }
            }
        }
        else
            add_reply(usr_id, _servername, ERR_NICKNAMEINUSE, client.get_nick(), nick,  MSG_NICKNAMEINUSE);
        
    }
    else
        add_info_reply(usr_id, _servername, ERR_NONICKNAMEGIVEN, client.get_nick(), MSG_NEEDMOREPARAMS);
}

void    Server::pass_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    Client &client = _clients.at(usr_id);
    if ( client.is_registered() )
        add_info_reply(usr_id, _servername, ERR_ALREADYREGISTRED, client.get_nick(), MSG_ALREADYREGISTRED);
    if (_pass.empty())
        client.set_pass_validity(true);
    else if ( args.size() == 1 )
         client.set_pass_validity(args.front() == _pass);
    else
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, client.get_nick(), "PASS", MSG_NEEDMOREPARAMS);
}

void    Server::part_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    Client      &client = _clients.at(usr_id);

    if (_parser.get_nargs() >= 1)
    {  
        std::string c_name  = args.front();
        std::string message = _parser.get_message();

        if (_channels.find(c_name) != _channels.end())
        {
            Channel &t_channel = _channels.at(c_name);
            if (t_channel.is_client(usr_id))
            {
                t_channel.part_client(usr_id);
                add_info_reply(usr_id, client.get_serv_id(), "PART", c_name, message);
            }
            else
                add_reply(usr_id, _servername, ERR_NOTONCHANNEL, client.get_nick(), c_name, MSG_NOTONCHANNEL);
        }
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, client.get_nick(), c_name, MSG_NOSUCHCHANNEL);
    }
    else
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, client.get_nick(), "PART", MSG_NEEDMOREPARAMS);
}

void    Server::ping_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    size_t nargs = _parser.get_nargs();

    if (nargs == 2)
    {
        std::cout << _servername << " " << args[1] << std::endl;
        if (_nicks.find(args[0]) == _nicks.end())
            add_info_reply(usr_id, _servername, ERR_NOSUCHSERVER, "PING", MSG_NOSUCHSERVER);
        else
        {
            Client &client = _clients.at(usr_id);
            add_info_reply(usr_id, _servername, "PONG", client.get_nick(), _servername);
        }
    }
}

std::string Server::who_information(Client &client, const std::string &channel_name)
{
    std::string prefix = "";
    std::string status = "H";
    std::string who_msg = "";
    if (channel_name != "*")
    {
        Channel &channel = _channels.at(channel_name);
        prefix = channel.get_member_prefix(client);
        prefix = (prefix == "~") ? "@" : prefix;
        if (channel.is_client_present(client) == false)
            status = "G";
    }
    who_msg = "~" + client.get_username() + " " + client.get_addr();
    who_msg += " " + _servername + " " + client.get_nick() + " ";
    who_msg += status + prefix + ":0" + " " + client.get_real_name();
    return who_msg;
}

void    Server::who_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    size_t nargs = _parser.get_nargs();
    Client &invitor = _clients.at(usr_id);

    if (nargs == 1)
    {
        std::string t_name = args.front();
        if (_nicks.find(t_name) != _nicks.end())
        {
            Client &client = _clients.at(_nicks.at(t_name));
            std::string channel_name = client.get_active_channel();
            std::string who_msg = who_information(client, t_name);
            add_reply(usr_id, _servername, RPL_WHOREPLY, client.get_nick(), channel_name, who_msg, false);
        }
        if (_channels.find(t_name) != _channels.end())
        {
            Channel& channel = _channels.at(t_name);
            std::map<int, Client &> clients = channel.get_clients();
            for (std::map<int, Client &>::iterator it = clients.begin(); it != clients.end(); it++)
            {
                Client &client = it->second;
                std::string who_msg = who_information(client, t_name);
                add_reply(usr_id, _servername, RPL_WHOREPLY, client.get_nick(), t_name, who_msg, false);
            }
        }
        add_reply(usr_id, _servername, RPL_ENDOFWHO, invitor.get_nick(), t_name, MSG_ENDOFWHO);
    }    
}

void    Server::invite_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    size_t nargs = _parser.get_nargs();
    Client &invitor = _clients.at(usr_id);

    if ( nargs >= 2 )
    {
        if (_nicks.find(args.at(0)) == _nicks.end())
            add_reply(usr_id, _servername, ERR_NOSUCHNICK, invitor.get_nick(), args.at(0), MSG_NOSUCHNICK);
        else if (_channels.find(args.at(1)) == _channels.end())
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, invitor.get_nick(), args.at(1), MSG_NOSUCHCHANNEL);
        else
        {
            Channel &channel = _channels.at(args.at(1));
            Client &invited = _clients.at(_nicks.at(args.at(0)));
            if (channel.is_client(invited.get_id()) == true)
                add_reply(usr_id, _servername, ERR_USERONCHANNEL, invitor.get_nick(), invited.get_nick(), MSG_USERONCHANNEL);
            else
            {
                if (channel.is_client_operator(invitor) == false && channel.is_channel_invite_only())
                    add_reply(usr_id, _servername, invitor.get_nick(), channel.get_name(), ERR_CHANOPRIVSNEEDED, MSG_CHANOPRIVSNEEDED);
                else
                {
                    channel.add_to_invites(invited);
                    add_reply(invitor.get_id(), _servername, RPL_INVITING, invitor.get_nick(), invited.get_nick(), channel.get_name());
                    add_reply(invited.get_id(), invitor.get_serv_id(), "INVITE", invited.get_nick(), channel.get_name());
                }
            }
        }
    }
    else
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, invitor.get_nick(), "INVITE", MSG_NEEDMOREPARAMS);
}

void    Server::kick_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    Client  &sender =   _clients.at(usr_id);
    size_t nargs = _parser.get_nargs();

    if ( nargs >= 2 )
    {
        std::string c_name   = args.front();
        std::string t_name   = args.at(1);
        std::string message  = _parser.get_message();
        if (_channels.find(c_name) != _channels.end())
        {
            Channel &target_channel =   _channels.at(c_name);

            if (is_nick_used(t_name) && target_channel.is_client(_nicks.at(t_name)))
            {
                if (target_channel.is_client(usr_id) == false)
                        add_reply(usr_id, _servername, ERR_NOTONCHANNEL, sender.get_nick(), t_name, MSG_NOTONCHANNEL);
                else if (target_channel.is_client_operator(sender) and t_name != target_channel.get_owner_nick())
                {
                    int target_id = _nicks.at(t_name);

                    target_channel.part_client(target_id);
                    if (message.empty())
                        message = t_name;
                    add_reply(target_id, sender.get_serv_id(), "KICK", c_name, t_name, message);
                    add_reply(usr_id, sender.get_serv_id(), "KICK", c_name, t_name, message);
                }
                else
                    add_reply(usr_id, _servername, ERR_CHANOPRIVSNEEDED, sender.get_nick(), c_name, MSG_CHANOPRIVSNEEDED);
            }
            else
                add_reply(usr_id, _servername, ERR_USERNOTINCHANNEL, sender.get_nick(), t_name, MSG_USERNOTINCHANNEL);
        }
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, sender.get_nick(), c_name, MSG_NOSUCHCHANNEL);
    }
    else
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, sender.get_nick(), "KICK", MSG_NEEDMOREPARAMS);
}

void    Server::topic_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    Client& client = _clients.at(usr_id);
    size_t nargs = _parser.get_nargs();

    if ( nargs >= 1 )
    {
        std::string c_name  = args.front();
        std::string topic   = _parser.get_message();
        bool has_topic      = _parser.get_has_message();
        if (_channels.find(c_name) != _channels.end())
        {
            Channel &channel = _channels.at(c_name);
            if (!channel.is_client(usr_id))
                add_reply(usr_id, _servername, ERR_NOTONCHANNEL, client.get_nick(), c_name, MSG_NOTONCHANNEL);
            else if (!has_topic)
            {
                if (channel.get_topic().size())
                    add_reply(usr_id, _servername, RPL_TOPIC, client.get_nick(), c_name, channel.get_topic());
                else
                    add_reply(usr_id, _servername, RPL_NOTOPIC, client.get_nick(), c_name, MSG_NOTOPIC);
            }
            else
            {
                if (channel.is_topic_lock() && channel.is_client_operator(client) == false)
                    add_reply(usr_id, _servername, ERR_CHANOPRIVSNEEDED, client.get_nick(), c_name, MSG_CHANOPRIVSNEEDED);
                else
                {
                    channel.set_topic(topic);
                    add_reply(usr_id, _servername, RPL_TOPIC, client.get_nick(), c_name, channel.get_topic());
                }
            }
        }
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, client.get_nick(), c_name, MSG_NOSUCHCHANNEL);
    }
    else
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, client.get_nick(), "TOPIC", MSG_NEEDMOREPARAMS);
}

void    Server::names_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    Client& client = _clients.at(usr_id);
    std::string usr_nick = client.get_nick();
    if (args.size() == 1)
    {
        std::string c_name = args.front();
        if (_channels.find(c_name) != _channels.end())
        {
            Channel &channel = _channels.at(c_name);
            if (channel.is_client(usr_id))
            {
                std::map<int, Client &> clients = channel.get_present_clients();
                std::string c_status = (channel.is_channel_secret() ? "@ " : "= ") + c_name;
                std::string names = "";
                for (std::map<int, Client &>::iterator it = clients.begin(); it != clients.end(); it++)
                {
                    if ( !(it->second.is_visible() || channel.is_client_operator(client) ||  it->second.get_nick() == usr_nick) )
                        continue;
                    if (it != clients.begin())
                        names += " ";
                    names += channel.get_member_prefix(it->second) + it->second.get_nick();
                }
                add_reply(usr_id, _servername, RPL_NAMREPLY, usr_nick, c_status, names);
                add_reply(usr_id, _servername, RPL_ENDOFNAMES, usr_nick, c_name, MSG_ENDOFNAMES);
            }
            else
                add_reply(usr_id, _servername, ERR_NOTONCHANNEL, usr_nick, c_name, MSG_NOTONCHANNEL);
        }
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, usr_nick, c_name, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() == 0)
    {
        // make a copy of _nicks keys in a vector called names
        std::vector<std::string> all_names;
        for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
        {
            if (it->second.is_visible() | it->second.get_nick() == usr_nick)
                all_names.push_back(it->second.get_nick());
        }
        for (std::map<const std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
        {
            Channel &channel = it->second;
            std::map<int, Client &> clients = channel.get_present_clients();
            if ( channel.is_channel_secret() == false || channel.is_client(usr_id) )
            {
                std::string c_status = (channel.is_channel_secret() ? "@ " : "= ") + it->first;
                std::string names = "";
                for (std::map<int, Client &>::iterator it2 = clients.begin(); it2 != clients.end(); it2++)
                {
                    if ( !(it2->second.is_visible() || channel.is_client_operator(client) ||  it2->second.get_nick() == usr_nick) )
                        continue;
                    if (it2 != clients.begin())
                        names += " ";
                    names += channel.get_member_prefix(it2->second) + it2->second.get_nick();
                    std::vector<std::string>::iterator name_index = std::find(all_names.begin(), all_names.end(), it2->second.get_nick());
                    if (name_index != all_names.end())
                        all_names.erase(name_index);
                }
                add_reply(usr_id, _servername, RPL_NAMREPLY, usr_nick, c_status, names);
            }
        }
        std::string names = "";
        for (std::vector<std::string>::iterator it = all_names.begin(); it != all_names.end(); it++)
            names += *it + " ";
        add_reply(usr_id, _servername, RPL_NAMREPLY, usr_nick, "* *", names);
        add_reply(usr_id, _servername, RPL_ENDOFNAMES, usr_nick, "*", MSG_ENDOFNAMES);
    }
}

void    Server::join_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    Client      &client = _clients.at(usr_id);
    size_t nargs = _parser.get_nargs();
    if ( nargs >= 1 )
    {
        std::string c_name  = args.front();
        std::string key     = "";

        if (nargs >= 2)
            key = args.at(1);
        if (_channels.find(c_name) == _channels.end())
        {
            std::string t_name = Channel::get_valid_channel_name(c_name);
            if (t_name.size() > 0)
            {
                _channels.insert(std::pair<std::string, Channel>(t_name, Channel(client, t_name)));
                client.join_channel(t_name);
                add_info_reply(usr_id, client.get_serv_id(), "JOIN", t_name);
                add_reply(usr_id, _servername, "MODE", t_name, _channels.at(t_name).get_modes_with_args());
                names_cmd(usr_id);
            }
            else
                add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, client.get_nick(), c_name, MSG_NOSUCHCHANNEL);
        }
        else
        {
            Channel &channel = _channels.at(c_name);
            if (channel.get_key() == key)
            {
                bool is_invited = channel.is_client_invited(client);
                if (is_invited == false)
                    add_reply(usr_id, _servername, ERR_INVITEONLYCHAN, client.get_nick(), c_name, MSG_INVITEONLYCHAN);
                else if (channel.is_there_space() == false)
                    add_reply(usr_id, _servername, ERR_CHANNELISFULL, client.get_nick(), c_name, MSG_CHANNELISFULL);
                else if (channel.is_client_banned(client) == false)
                {
                    if (is_invited == true)
                        channel.remove_from_invites(client.get_id());
                    channel.add_client(client);
                    client.join_channel(c_name);
                    add_info_reply(usr_id, client.get_serv_id(), "JOIN", c_name);
                    add_reply(usr_id, _servername, "MODE", c_name, channel.get_modes_with_args());
                    if (channel.get_topic().size())
                        topic_cmd(usr_id);
                    names_cmd(usr_id);
                }
                else
                    add_reply(usr_id, _servername, ERR_BANNEDFROMCHAN, client.get_nick(), c_name, MSG_BANNEDFROMCHAN);
            }
            else
                add_reply(usr_id, _servername, ERR_BADCHANNELKEY, client.get_nick(), c_name, MSG_BADCHANNELKEY);
        }
    }
    else
        add_info_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, client.get_nick(), MSG_NEEDMOREPARAMS);
}

void    Server::handle_commands(int usr_id, std::string &command)
{
    _parser.parse(command);
    std::string command_name = _parser.get_command();
    if ( _commands.find(command_name) != _commands.end())
    {
        Client &client = _clients.at(usr_id);
        if ( client.get_status() == Client::UNREGISTERED )
        {
            if (command_name == "QUIT")
                (this->*_commands[command_name])(usr_id);
            else if (command_name == "PASS")
            {
                if (client.get_pass_validity() == false)
                    (this->*_commands[command_name])(usr_id);
                client.update_registration();
            }
            else if ((command_name == "USER" || command_name == "NICK"))
            {
                if (client.get_pass_validity() == true)
                    (this->*_commands[command_name])(usr_id);
                client.update_registration();
            }
            else
                add_reply(usr_id, _servername, ERR_NOTREGISTERED, client.get_nick(), command_name, MSG_NOTREGISTERED);
            if (client.get_status() == Client::REGISTERED)
                add_info_reply(usr_id, _servername, RPL_WELCOME, client.get_nick(), _motd);
        }
        else if ( client.get_status() == Client::REGISTERED )
        {
            if ( command_name == "PASS" ||  command_name == "USER" )
                add_reply(usr_id, _servername, ERR_ALREADYREGISTRED, client.get_nick(), command_name, MSG_ALREADYREGISTRED);
            else
                (this->*_commands[command_name])(usr_id);
        }
    }
}

void    Server::print_msg(int fd)
{
    static char msg_buffer[MAX_COMMAND_SIZE];
    size_t      msg_len;
    std::string command;

    if ( (msg_len = recv(fd, msg_buffer, MAX_COMMAND_SIZE, MSG_DONTWAIT)) > 0)
    {
        msg_buffer[msg_len] = '\0';
        std::cout << "MSG RECVED:" << msg_buffer << std::endl;
        _clients.at(fd).add_command(std::string(msg_buffer));
    }
    while((command = _clients.at(fd).get_command()) != "")
        handle_commands(fd, command);
}

void    Server::send_replies()
{
    while (!_replies.empty())
    {
        int fd = _replies.front().first;
        std::string reply = _replies.front().second;
        if (send(fd, reply.c_str(), reply.length(), 0) > 0)
            _replies.pop();
        if (_clients.at(fd).get_status() == Client::DOWN)
            remove_connection(fd);
    }
}

void    Server::start()
{
    while (true)
    {
        if (poll(_pfds, _nfds, -1) < 0)
            continue;
        for (int i = 0; i < _nfds; i++)
        {
#if defined(__linux__)
            if (_pfds[i].revents & POLLRDHUP)
#elif defined(__APPLE__)
            if (_pfds[i].revents & POLLHUP)
#endif
                remove_connection(_pfds[i].fd);
            else if (_pfds[i].revents & POLLIN)
            {
                if (_pfds[i].fd != _sockfd)
                    print_msg(_pfds[i].fd);
                else
                    accept_connection();
            }
        }
        send_replies();
    }
}

Server* Server::getInstance()
{
    return Server::_instance;
};

void    Server::deleteInstance()
{
    if (Server::_instance != NULL)
    {
        delete Server::_instance;
        Server::_instance = NULL;
    }
}

void     Server::initServer(const char *port, const char *pass)
{
    if (Server::_instance == NULL)
        Server::_instance = new Server(port, pass);
    else
        throw Server::ServerException("Server already initialized.");
};