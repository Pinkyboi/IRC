#include "Parser.hpp"

Parser::Parser() {}
Parser::~Parser() {}

std::vector<std::string>    split_command(std::string message, std::string sep)
{
    std::vector<std::string>    tokens;
    std::string                 token;
    size_t                      i, j;
    size_t                      f, l;

    j = 0;
    for (i = 0; j != std::string::npos; i = j + 1)
    {
        j = message.find_first_of(sep, i);
        token = message.substr(i, j - i);
        if (token.size())
            tokens.push_back(token);
    }
    return tokens;
}

std::string &Parser::getCommand()
{
    return _command;
}

std::vector<std::string> &Parser::getArguments()
{
    return _arguments;
}

std::string &Parser::getMessage()
{
    return _message;
}

void    Parser::parse(std::string message)
{
    std::vector<std::string>    tokens;
    _command.clear();
    _arguments.clear();
    _message.clear();

    // Find the message part.
    size_t pos = message.find_first_of(":");
    if (pos != std::string::npos)
    {
        _message = message.substr(pos+1, message.size() - pos);
        message = message.substr(0, pos);
    }

    // Find the command and parameters part.
    tokens = split_command(message, " ");
    if (tokens.empty())
        return ;
    _command = std::string(tokens[0]);
    for (size_t i = 1; i < tokens.size(); i++)
        _arguments.push_back(std::string(tokens[i]));
}
