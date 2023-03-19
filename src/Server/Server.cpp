#include "Server.hpp"

Server *Server::_instance = NULL;
std::string Server::_servername = "superDuperServer";
std::string Server::_motd = "this is the message of the day";

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
    _commands.insert(std::pair<std::string, cmd_func>("QUIT", &Server::quit_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("MODE", &Server::mode_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("INVITE", &Server::invite_cmd));
    _commands.insert(std::pair<std::string, cmd_func>("PING", &Server::ping_cmd));
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

    add_reply(usr_id, _servername, "QUIT", "", message);
    _clients.at(usr_id).set_status(Client::DOWN);
}

void    Server::privmsg_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    std::string message = _parser.get_message();
    Client& client = _clients.at(usr_id);
    std::string s_name = client.get_serv_id();
    if (message.size() == 0)
        add_reply(usr_id, _servername, ERR_NOTEXTTOSEND, s_name, MSG_NOTEXTTOSEND);
    if (args.size() == 1)
    {
        std::string t_name = args.front();
        if (_channels.find(t_name) != _channels.end())
        {
            Channel &t_channel = _channels.at(t_name);

            if (t_channel.is_channel_client_only() && t_channel.is_client(usr_id) == false)
                add_reply(usr_id, _servername, ERR_CANNOTSENDTOCHAN, t_name, MSG_CANNOTSENDTOCHAN);
            else if (t_channel.is_client_unmute(client) == false)
                add_reply(usr_id, _servername, ERR_CANNOTSENDTOCHAN, t_name, MSG_CANNOTSENDTOCHAN);
            else if (!t_channel.is_client_banned(_clients.at(usr_id)))
            {
                std::map<int, Client&> &clients = t_channel.get_clients();
                for (std::map<int, Client&>::iterator it = clients.begin(); it != clients.end(); it++)
                    add_reply(it->first, s_name, RPL_PRIVMSG, it->second.get_nick(), message);
            }
            else
                add_reply(usr_id, _servername, ERR_BANNEDFROMCHAN, t_name, MSG_BANNEDFROMCHAN);
        }
        else if (is_nick_used(t_name))
            add_reply(_nicks.at(t_name), s_name, RPL_PRIVMSG, t_name, message);
        else
            add_reply(usr_id, _servername, ERR_NOSUCHNICK, "PRIVMSG", MSG_NOSUCHNICK);
    }
    else if (args.size() > 1)
        add_reply(usr_id, _servername, ERR_TOOMANYTARGETS, "PRIVMSG", MSG_TOOMANYTARGETS);
    else
        add_reply(usr_id, _servername, ERR_NORECIPIENT, "PRIVMSG", MSG_NORECIPIENT);
}


void    Server::mode_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    std::string message = _parser.get_message();
    size_t nargs = _parser.get_nargs();
    Client &client  = _clients.at(usr_id);

    if ( nargs == 2 || nargs == 3 )
    {
        std::string t_name = args.front();
        std::string argument = "";
        std::string modes = args.at(1);
        if (nargs == 3)
            argument = args.back();
        if (_nicks.find(t_name) != _nicks.end())
        {
            if (client.get_nick() == t_name)
            {
                if (client.handle_modes(modes))
                    add_reply(usr_id, _servername, "MODE", client.get_modes());
            }
            else
                add_reply(usr_id, _servername, ERR_USERSDONTMATCH, "MODE", MSG_USERSDONTMATCH);
        }
        else if (_channels.find(t_name) != _channels.end())
        {
            if (_channels.at(t_name).is_client_operator(client))
            {
                Channel& t_channel =  _channels.at(t_name);
                if (t_channel.handle_modes(modes, argument))
                    add_reply(usr_id, _servername, "MODE", t_name, t_channel.get_modes_with_args());
            }
            else
                add_reply(usr_id, _servername, ERR_CHANOPRIVSNEEDED, "MODE", MSG_CHANOPRIVSNEEDED);
        }
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, t_name, MSG_NOSUCHCHANNEL);
    }
    else if (nargs == 1)
    {
        std::string t_name = args.front();
        if (_nicks.find(t_name) != _nicks.end())
        {
            if (client.get_nick() == t_name)
                add_reply(usr_id, _servername, RPL_UMODEIS, client.get_modes());
            else
                add_reply(usr_id, _servername, ERR_USERSDONTMATCH, "MODE", MSG_USERSDONTMATCH);
        }
        else if (_channels.find(t_name) != _channels.end())
            add_reply(usr_id, _servername, RPL_CHANNELMODEIS, t_name, _channels.at(t_name).get_modes_with_args());
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, t_name, MSG_NOSUCHCHANNEL);
    }
    else
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, "MODE", MSG_NEEDMOREPARAMS);
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
            std::map<int, Client&> &clients = _channels.at(t_name).get_clients();
            for (std::map<int, Client&>::iterator it = clients.begin(); it != clients.end(); it++)
                add_reply(it->first, s_name, RPL_PRIVMSG, it->second.get_nick(), message);
        }
        else if (is_nick_used(t_name))
            add_reply(_nicks.at(t_name), _servername, RPL_PRIVMSG, s_name, message);
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
        add_reply(usr_id, _servername, RPL_LISTSTART, nick, MSG_LISTSTART);
        for (std::map<const std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
        {
            if (it->second.is_channel_secret() == false || it->second.is_client(usr_id))
            {
                std::string msg = it->first + " " + convert_to_string(it->second.get_clients_count()) + " " + it->second.get_topic();
                add_reply(usr_id, _servername, RPL_LIST, nick, msg, false);
            }
        }
        add_reply(usr_id, _servername, RPL_LISTEND, nick, MSG_LISTEND);
    }
    else if (args.size() >= 1)
    {
        std::string c_name = args.at(0);
        if (_channels.find(c_name) != _channels.end())
        {
            Channel &channel = _channels.at(c_name);
            if (channel.is_channel_secret() == false || channel.is_client(usr_id))
            {
                std::string msg = c_name + " " + convert_to_string(channel.get_clients_count()) + " " + channel.get_topic();
                add_reply(usr_id, _servername, RPL_LIST, nick, msg);
            }
        }
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, c_name, MSG_NOSUCHCHANNEL);
        add_reply(usr_id, _servername, RPL_LISTEND, "LIST", MSG_LISTEND);
    }
}

void    Server::add_reply(int usr_id, const std::string &sender, const std::string &code, const std::string &target, const std::string &extra, bool is_msg)
{
    std::string replymsg = ":" + sender + " " + code + " " + target;
    if (extra.size())
    {
        replymsg += " ";
        if (is_msg)
            replymsg += ":";
    }
    replymsg += extra + CRLN;
    std::cout << replymsg << std::endl;
    _replies.push(std::pair<int, std::string>(usr_id, replymsg));
}

void    Server::user_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    std::string real_name = _parser.get_message();

    if (args.size() != 3 || real_name.empty())
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, _clients.at(usr_id).get_nick(), MSG_NEEDMOREPARAMS);
    else if (_clients.at(usr_id).is_registered())
        add_reply(usr_id, _servername, ERR_ALREADYREGISTRED, _clients.at(usr_id).get_nick(), MSG_ALREADYREGISTRED);
    else
    {
        std::string username = args.front();
        std::string mode = args.at(1);
        _clients.at(usr_id).set_username(username);
        _clients.at(usr_id).set_real_name(real_name);
        _clients.at(usr_id).set_mode(mode);
    }
}

void    Server::update_nick(int usr_id, std::string newnick)
{
    std::string oldnick = _clients.at(usr_id).get_nick();
    _clients.at(usr_id).set_nick(newnick);
    _nicks.erase(oldnick);
    _nicks.insert(std::pair<std::string, int>(newnick, usr_id));
}

void    Server::add_nick(int usr_id, std::string nick)
{
    _nicks.insert(std::pair<std::string, int>(nick, usr_id));
}

void    Server::nick_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();

    if ( args.size() == 1 )
    {
        std::string nick  = args.front();
        Client &client = _clients.at(usr_id);
        if ( is_nick_used(nick) ==  false )
        {
            if (client.is_registered())
                update_nick(usr_id, nick);
            else
            {
                if (Client::is_nick_valid(nick) == false)
                    add_reply(usr_id, _servername, ERR_ERRONEUSNICKNAME, "NICK", MSG_ERRONEUSNICKNAME);
                else
                {
                    client.set_nick(nick);
                    add_nick(usr_id, nick);
                }
            }
        }
        else
            add_reply(usr_id, _servername, ERR_NICKNAMEINUSE, "NICK",  MSG_NICKNAMEINUSE);
        
    }
    else
        add_reply(usr_id, _servername, ERR_NONICKNAMEGIVEN, "NICK", MSG_NEEDMOREPARAMS);
}

void    Server::pass_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    Client &client = _clients.at(usr_id);
    if ( client.is_registered() )
        add_reply(usr_id, _servername, ERR_ALREADYREGISTRED, client.get_nick(), MSG_ALREADYREGISTRED);
    if (_pass.empty())
        client.set_pass_validity(true);
    else if ( args.size() == 1 )
         client.set_pass_validity(args.front() == _pass);
    else
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, "PASS", MSG_NEEDMOREPARAMS);
}

void    Server::part_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();

    if (_parser.get_nargs() == 1)
    {  
        std::string c_name  = args.front();
        std::string message = _parser.get_message();
        Client      &client = _clients.at(usr_id);

        if (_channels.find(c_name) != _channels.end())
        {
            if (_channels.at(c_name).is_client(usr_id))
            {
                _channels.at(c_name).remove_client(usr_id);
                if (message != "")
                    privmsg_cmd(usr_id);
                client.remove_channel(c_name);
                add_reply(usr_id, client.get_serv_id(), "PART", c_name);
            }
            else
                add_reply(usr_id, _servername, ERR_NOTONCHANNEL, _clients.at(usr_id).get_nick(), MSG_NOTONCHANNEL);
        }
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, c_name, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() == 0)
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, "PART", MSG_NEEDMOREPARAMS);
}

void    Server::ping_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    size_t nargs = _parser.get_nargs();

    if (nargs == 2)
    {
        std::cout << _servername << " " << args[1] << std::endl;
        if (_nicks.find(args[0]) == _nicks.end())
            add_reply(usr_id, _servername, ERR_NOSUCHSERVER, "PING", MSG_NOSUCHSERVER);
        else
        {
            Client &client = _clients.at(usr_id);
            add_reply(usr_id, _servername, "PONG", client.get_nick(), _servername);
        }
    }
}

void    Server::invite_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    size_t nargs = _parser.get_nargs();
    Client &invitor = _clients.at(usr_id);

    if ( nargs == 2 )
    {
        if (_nicks.find(args.at(0)) == _nicks.end())
            add_reply(usr_id, _servername, ERR_NOSUCHNICK, _clients.at(usr_id).get_nick(), MSG_NOSUCHNICK);
        else if (_channels.find(args.at(1)) == _channels.end())
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, args.at(1), MSG_NOSUCHCHANNEL);
        else
        {
            Channel &channel = _channels.at(args.at(1));
            Client &client = _clients.at(_nicks.at(args.at(0)));
            if (channel.is_client(client.get_id()) == true)
                add_reply(usr_id, _servername, ERR_USERONCHANNEL, client.get_nick(), MSG_USERONCHANNEL);
            else
            {
                if (channel.is_client_operator(invitor) == false && channel.is_channel_invite_only())
                    add_reply(usr_id, _servername, invitor.get_nick(), ERR_CHANOPRIVSNEEDED, MSG_CHANOPRIVSNEEDED);
                else
                {
                    channel.add_to_invites(client);
                    add_reply(invitor.get_id(), _servername, RPL_INVITING, client.get_nick(), channel.get_name(), false);
                    add_reply(client.get_id(), _servername, "INVITE", client.get_nick(), channel.get_name(), false);
                }
            }
        }
    }
    else if ( nargs < 2 )
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, "INVITE", MSG_NEEDMOREPARAMS);
}

void    Server::kick_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();

    if ( _parser.get_nargs() == 2 )
    {
        std::string c_name   = args.front();
        std::string t_name   = args.at(1);
        std::string message  = _parser.get_message();
        if (_channels.find(c_name) != _channels.end())
        {
            Channel &target_channel =   _channels.at(c_name);
            Client  &sender         =   _clients.at(usr_id);
            if (is_nick_used(t_name) && target_channel.is_client(_nicks.at(t_name)))
            {
                if (target_channel.is_client(usr_id) == false)
                        add_reply(usr_id, _servername, ERR_NOTONCHANNEL, sender.get_nick(), MSG_NOTONCHANNEL);
                else if (target_channel.is_client_operator(sender) and t_name != target_channel.get_owner_nick())
                {
                    int target_id = _nicks.at(t_name);
                    target_channel.remove_client(target_id);
                    _clients.at(target_id).remove_channel(c_name);
                    add_reply(target_id, sender.get_serv_id(), RPL_PRIVMSG, t_name, message);
                }
                else
                    add_reply(usr_id, _servername, ERR_CHANOPRIVSNEEDED, sender.get_nick(), MSG_CHANOPRIVSNEEDED);
            }
            else
                add_reply(usr_id, _servername, ERR_USERNOTINCHANNEL, sender.get_nick(), MSG_USERNOTINCHANNEL);
        }
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, c_name, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() < 2)
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, "KICK", MSG_NEEDMOREPARAMS);
}

void    Server::topic_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    Client& client = _clients.at(usr_id);
    if ( args.size() == 1 )
    {
        std::string c_name  = args.front();
        std::string topic   = _parser.get_message();
        bool has_topic      = _parser.get_has_message();
        if (_channels.find(c_name) != _channels.end())
        {
            Channel &channel = _channels.at(c_name);
            if (!channel.is_client(usr_id))
                add_reply(usr_id, _servername, ERR_NOTONCHANNEL, c_name, MSG_NOTONCHANNEL);
            else if (!has_topic)
            {
                if (!_channels.at(c_name).get_topic().empty())
                    add_reply(usr_id, _servername, RPL_TOPIC, c_name, _channels.at(c_name).get_topic());
                else
                    add_reply(usr_id, _servername, RPL_NOTOPIC, c_name, MSG_NOTOPIC);
            }
            else
            {
                if (channel.is_topic_lock() && channel.is_client_operator(client) == false)
                    add_reply(usr_id, _servername, ERR_CHANOPRIVSNEEDED, c_name, MSG_CHANOPRIVSNEEDED);
                else
                    _channels.at(c_name).set_topic(topic);
            }
        }
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, c_name, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() == 0)
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, "TOPIC", MSG_NEEDMOREPARAMS);
}

void    Server::names_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    if (args.size() == 1)
    {
        std::string c_name = args.front();
        if (_channels.find(c_name) != _channels.end())
        {
            Channel &channel = _channels.at(c_name);
            if (channel.is_client(usr_id))
            {
                std::map<int, Client &> clients = channel.get_clients();
                std::string names = "";
                if (channel.is_channel_secret())
                    c_name = "@ " + c_name;
                else
                    c_name = "= " + c_name;
                for (std::map<int, Client &>::iterator it = clients.begin(); it != clients.end(); it++)
                {
                    if (it->second.is_visible() == false && channel.is_client_operator(_clients.at(usr_id)) == false)
                        continue;
                    if (it != clients.begin())
                        names += " ";
                    if (channel.is_client_owner(it->second))
                        names += "~";
                    else if (channel.is_client_operator(it->second))
                        names += "@";
                    else if (channel.is_client_unmute(it->second))
                        names += "+";
                    names += it->second.get_nick();
                }
                add_reply(usr_id, _servername, RPL_NAMREPLY, c_name, names);
                add_reply(usr_id, _servername, RPL_ENDOFNAMES, "NAMES", MSG_ENDOFNAMES);
            }
            else
                add_reply(usr_id, _servername, ERR_NOTONCHANNEL, c_name, MSG_NOTONCHANNEL);
        }
        else
            add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, c_name, MSG_NOSUCHCHANNEL);
    }
    else if (args.size() == 0)
    {
        // make a copy of _nicks keys in a vector called names
        std::vector<std::string> all_names;
        for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
        {
            if (it->second.is_visible())
                all_names.push_back(it->second.get_nick());
        }
        // loop throught he channels and remove the name found in channel from all names
        for (std::map<const std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
        {
            std::map<int, Client &> clients = it->second.get_clients();
            std::string c_name = it->first;
            if (it->second.is_channel_secret())
                c_name = "@ " + c_name;
            else
                c_name = "= " + c_name;
            if ( it->second.is_channel_secret() == false || it->second.is_client(usr_id) )
            {
                std::string names = "";
                for (std::map<int, Client &>::iterator it2 = clients.begin(); it2 != clients.end(); it2++)
                {
                    if (it2->second.is_visible() == false)
                        continue;
                    if (it2 != clients.begin())
                        names += " ";
                    if (it->second.is_client_owner(it2->second))
                        names += "~";
                    if (it->second.is_client_operator(it2->second))
                        names += "@";
                    else if (it->second.is_client_unmute(it2->second) && it->second.is_channel_moderated())
                        names += "+";
                    names += it2->second.get_nick();
                    std::vector<std::string>::iterator name_index = std::find(all_names.begin(), all_names.end(), it2->second.get_nick());
                    if (name_index != all_names.end())
                        all_names.erase(name_index);
                }
                add_reply(usr_id, _servername, RPL_NAMREPLY, it->first, names);
            }
        }
        std::string names = "";
        for (std::vector<std::string>::iterator it = all_names.begin(); it != all_names.end(); it++)
            names += *it + " ";
        if (names.size() > 0)
            add_reply(usr_id, _servername, RPL_NAMREPLY, "*", names);
        add_reply(usr_id, _servername, RPL_ENDOFNAMES, "*", MSG_ENDOFNAMES);
    }
}

void    Server::join_cmd(int usr_id)
{
    std::vector<std::string> args = _parser.get_arguments();
    if ( args.size() == 1 || args.size() == 2 )
    {
        std::string c_name  = args.front();
        std::string key     = "";
        Client      &client = _clients.at(usr_id);

        if (args.size() == 2)
            key = args.at(1);
        if (_channels.find(c_name) == _channels.end())
        {
            c_name = Channel::get_valid_channel_name(c_name);
            if (c_name.size() > 0)
            {
                _channels.insert(std::pair<std::string, Channel>(c_name, Channel(client, c_name)));
                add_reply(usr_id, _clients.at(usr_id).get_serv_id(), "JOIN", c_name);
                topic_cmd(usr_id);
                names_cmd(usr_id);
            }
            else
                add_reply(usr_id, _servername, ERR_NOSUCHCHANNEL, c_name, MSG_NOSUCHCHANNEL);
        }
        else
        {
            Channel &channel = _channels.at(c_name);
            if (channel.get_key() == key)
            {
                bool is_invited = channel.is_client_invited(client);
                if (is_invited == false)
                    add_reply(usr_id, _servername, ERR_INVITEONLYCHAN, c_name, MSG_INVITEONLYCHAN);
                else if (channel.is_there_space() == false)
                    add_reply(usr_id, _servername, ERR_CHANNELISFULL, c_name, MSG_CHANNELISFULL);
                else if (channel.is_client_banned(client) == false)
                {
                    if (is_invited == true)
                        channel.remove_from_invites(client.get_id());
                    channel.add_client(client);
                    client.add_channel(c_name);
                    add_reply(usr_id, _clients.at(usr_id).get_serv_id(), "JOIN", c_name);
                    topic_cmd(usr_id);
                    names_cmd(usr_id);
                }
                else
                    add_reply(usr_id, _servername, ERR_BANNEDFROMCHAN, c_name, MSG_BANNEDFROMCHAN);
            }
            else
                add_reply(usr_id, _servername, ERR_BADCHANNELKEY, c_name, MSG_BADCHANNELKEY);
        }
    }
    else if (args.size() < 1)
        add_reply(usr_id, _servername, ERR_NEEDMOREPARAMS, "JOIN", MSG_NEEDMOREPARAMS);
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
                if (client.get_pass_validity() == false )
                    (this->*_commands[command_name])(usr_id);
                _clients.at(usr_id).update_registration();
            }
            else if ((command_name == "USER" || command_name == "NICK"))
            {
                if (client.get_pass_validity() == true)
                    (this->*_commands[command_name])(usr_id);
                _clients.at(usr_id).update_registration();
            }
            else
                add_reply(usr_id, _servername, ERR_NOTREGISTERED, client.get_nick(), MSG_NOTREGISTERED);
            if (client.get_status() == Client::REGISTERED)
                add_reply(usr_id, _servername, RPL_WELCOME, client.get_nick(), _motd);
        }
        else if ( client.get_status() == Client::REGISTERED )
        {
            if ( command_name == "PASS" ||  command_name == "USER" )
                add_reply(usr_id, _servername, ERR_ALREADYREGISTRED, client.get_nick(), MSG_ALREADYREGISTRED);
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
        std::cout << msg_buffer << std::endl;
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