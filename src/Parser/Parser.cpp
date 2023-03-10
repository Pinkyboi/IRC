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
    std::vector<std::string>    tokens;
    std::string                 token;
    size_t                      i, j;
    size_t                      f, l;

    message = strip(message);
    j = 0;
    for (i = 0; j != std::string::npos && times; i = j + 1)
    {
        j = message.find_first_of(sep, i);
        token = message.substr(i, j - i);
        if (token.size())
            tokens.push_back(token);
        times = times == -1 ? times : --times;
    }
    if (times == 0)
    {
        token = message.substr(i, message.size() - i);
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

    tokens = split_command(message, ":", 1);
    std::cout << tokens.size() << std::endl;
    if (tokens.size())
    {
        _message = std::string(tokens[tokens.size() - 1]);
        tokens = split_command(tokens[0], " ", -1);
        _command = std::string(tokens[0]);
        for (size_t i = 1; i < tokens.size(); i++)
            _arguments.push_back(std::string(tokens[i]));
    }
}
