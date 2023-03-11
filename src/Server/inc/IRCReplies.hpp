#ifndef IRC_REPLIES_HPP
#define IRC_REPLIES_HPP

#define RPL_WELCOME "001"
#define RPL_YOURHOST "002"
#define RPL_CREATED "003"
#define RPL_MYINFO "004"
#define RPL_ISUPPORT "005"
#define RPL_BOUNCE "010"
#define RPL_USERHOST "302"
#define RPL_ISON "303"
#define RPL_AWAY "301"
#define RPL_UNAWAY "305"
#define RPL_NOWAWAY "306"
#define RPL_WHOISUSER "311"
#define RPL_WHOISSERVER "312"
#define RPL_WHOISOPERATOR "313"
#define RPL_WHOISIDLE "317"
#define RPL_ENDOFWHOIS "318"
#define RPL_WHOISCHANNELS "319"
#define RPL_WHOWASUSER "314"
#define RPL_ENDOFWHOWAS "369"
#define RPL_LISTSTART "321"
#define RPL_LIST "322"
#define RPL_LISTEND "323"
#define RPL_UNIQOPIS "325"
#define RPL_CHANNELMODEIS "324"
#define RPL_NOTOPIC "331"
#define RPL_TOPIC "332" // "332" - Topic message
#define RPL_TOPICINFO "333" // "333" - Topic information
#define RPL_INVITING "341" // "341" - Invitation message
#define RPL_SUMMONING "342" // "342" - Summon message

#define RPL_VERSION "351" // "351" - Version message
#define RPL_WHOREPLY "352" // "352" - WHO message reply
#define RPL_NAMREPLY "353" // "353" - NAMES message reply
#define RPL_LINKS "364" // "364" - Links message
#define RPL_ENDOFLINKS "365" // "365" - End of links message
#define RPL_ENDOFNAMES "366" // "366" - End of NAMES message
#define RPL_BANLIST "367" // "367" - Ban list message
#define RPL_ENDOFBANLIST "368" // "368" - End of ban list message
#define RPL_ENDOFWHOWAS "369" // "369" - End of WHOWAS message
#define RPL_INFO "371" // "371" - Server information message
#define RPL_MOTD "372" // "372" - MOTD message
#define RPL_ENDOFINFO "374" // "374" - End of server information message
#define RPL_MOTDSTART "375" // "375" - Start of MOTD message
#define RPL_ENDOFMOTD "376" // "376" - End of MOTD message
#define RPL_YOUREOPER "381" // "381" - You're now an IRC operator
#define RPL_REHASHING "382" // "382" - Rehash message
#define RPL_YOURESERVICE "383" // "383" - You're now a service
#define RPL_TIME "391" // "391" - Time message
#define RPL_USERSSTART "392" // "392" - Start of users list message
#define RPL_USERS "393" // "393" - Users list message
#define RPL_ENDOFUSERS "394" // "394" - End of users list message
#define RPL_NOUSERS "395" // "395" - No users on the server message

#define ERR_NOSUCHNICK "401" // "401" - No such nickname message
#define ERR_NOSUCHSERVER "402" // "402" - No such server message
#define ERR_NOSUCHCHANNEL "403" // "403" - No such channel message
#define ERR_CANNOTSENDTOCHAN "404" // "404" - Cannot send to channel message
#define ERR_TOOMANYCHANNELS "405" // "405" - Too many channels message
#define ERR_WASNOSUCHNICK "406" // "406" - Was no such nickname message
#define ERR_TOOMANYTARGETS "407" // "407" - Too many targets message
#define ERR_NOORIGIN "409" // "409" - No origin message
#define ERR_NORECIPIENT "411" // "411" - No recipient message
#define ERR_NOTEXTTOSEND "412" // "412" - No text to send message
#define ERR_NOTOPLEVEL "413" // "413" - Not a top level domain message
#define ERR_WILDTOPLEVEL "414" // "414" - Wildcard in top level domain message
#define ERR_UNKNOWNCOMMAND "421" // "421" - Unknown command message
#define ERR_NOMOTD "422" // "422" - No MOTD message
#define ERR_NOADMININFO "423" // "423" - No admin info message
#define ERR_FILEERROR "424" // "424" - File error message
#define ERR_NONICKNAMEGIVEN "431" // "431" - No nickname given message
#define ERR_ERRONEUSNICKNAME "432" // "432" - Erroneous nickname message
#define ERR_NICKNAMEINUSE "433" // "433" - Nickname already in use message
#define ERR_NICKCOLLISION "436" // "436" - Nickname collision message
#define ERR_USERNOTINCHANNEL "441" // "441" - User not in channel message
#define ERR_NOTONCHANNEL "442" // "442" - Not on channel message
#define ERR_USERONCHANNEL "443" // "443" - User on channel message
#define ERR_NOLOGIN "444" // "444" - Not logged in message
#define ERR_SUMMONDISABLED "445" // "445" - Summon has been disabled message
#define ERR_USERSDISABLED "446" // "446" - Users have been disabled message
#define ERR_NOTREGISTERED "451" // "451" - Not registered message
#define ERR_NEEDMOREPARAMS "461" // "461" - Need more parameters message
#define ERR_ALREADYREGISTRED "462" // "462" - Already registered message
#define ERR_NOPERMFORHOST "463" // "463" - No permission for host message
#define ERR_PASSWDMISMATCH "464" // "464" - Password incorrect message
#define ERR_YOUREBANNEDCREEP "465" // "465" - You are banned message
#define ERR_YOUWILLBEBANNED "466" // "466" - You will be banned message
#define ERR_KEYSET "467" // "467" - Channel key already set message
#define ERR_CHANNELISFULL "471" // "471" - Channel is full message
#define ERR_UNKNOWNMODE "472" // "472" - Unknown mode message
#define ERR_INVITEONLYCHAN "473" // "473" - Invite only channel message
#define ERR_BANNEDFROMCHAN "474" // "474" - Banned from channel message
#define ERR_BADCHANNELKEY "475" // "475" - Bad channel key message
#define ERR_NOPRIVILEGES "481" // "481" - No privileges message
#define ERR_CHANOPRIVSNEEDED "482" // "482" - Channel operator privileges needed message
#define ERR_CANTKILLSERVER "483" // "483" - Cannot kill server message
#define ERR_RESTRICTED "484" // "484" - Your connection is restricted message
#define ERR_UNIQOPPRIVSNEEDED "485" // "485" - Unique operator privileges needed message
#define ERR_NOOPERHOST "491" // "491" - No IRC operator host message
#define ERR_UMODEUNKNOWNFLAG "501" // "501" - Unknown mode flag message
#define ERR_USERSDONTMATCH "502" // "502" - Users do not match message

#define RPL_PRIVMSG "PRIVMSG" // "PRIVMSG" - Private message

#define MSG_NEEDMOREPARAMS "Need more parameters"
#define MSG_ALREADYREGISTRED "You are already registered"
#define MSG_NICKNAMEINUSE "Nickname already in use"
#define MSG_YOUREOPER "You are now an IRC operator"
#define MSG_NOSUCHCHANNEL "No such channel"
#define MSG_NOTONCHANNEL "You are not on that channel"
#define MSG_USERNOTINCHANNEL "User is not in channel"
#define MSG_NOTOPIC "No topic is set"
#define MSG_LISTEND "End of LIST"
#define MSG_TOOMANYTARGETS "Too many targets"
#endif