#include "Parser.hpp"

Parser::Parser() {}
Parser::~Parser() {}

/*
<message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
<prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
<command>  ::= <letter> { <letter> } | <number> <number> <number>
<SPACE>    ::= ' ' { ' ' }
<params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]

<middle>   ::= <Any *non-empty* sequence of octets not including SPACE
               or NUL or CR or LF, the first of which may not be ':'>
<trailing> ::= <Any, possibly *empty*, sequence of octets not including
                 NUL or CR or LF>

<crlf>     ::= CR LF
*/

std::vector<std::string>    split_command(std::string message)
{
    size_t                      sep;    
    size_t                      i;
    std::vector<std::string>    tokens;
    std::string                 token;

    sep = 0;
    for (i = 0; sep != std::string::npos; i = sep + 1)
    {
        sep = message.find_first_of(" \0", i);
        if ((token = message.substr(i, sep - i)).size() == 0)
            tokens.push_back(token);
    }
    return tokens;
}

void    Parser::parse(std::string message)
{
    std::vector<std::string>    tokens = split_message(message);
    size_t size;

    size = tokens.size();
    if (size > 0)
    {
        if (tokens[0].compare("PASS") && size >= 1)
        {
            
        }
    }
}
