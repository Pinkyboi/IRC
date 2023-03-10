#include "Parser.hpp"

Parser::Parser() {}
Parser::~Parser() {}

static std::string strip(std::string str)
{
    size_t  f, l;
    f = str.find_first_not_of(" \0");
    l = str.find_last_not_of(" \0");
    return str.substr(f, l-f+1);
}

std::vector<std::string>    split_command(std::string message, std::string sep, size_t times)
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
    std::vector<std::string>    tokens = split_command(message);
    size_t size;

    size = tokens.size();
    if (size > 0)
    {
        if (tokens[0].compare("PASS") && size >= 1)
        {
            
        }
    }
}
