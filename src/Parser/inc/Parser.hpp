#ifndef PARSER_H
# define PARSER_H

# include <string>
# include <vector>
# include <iostream>

class Parser
{
    public:
        Parser();
        ~Parser();
        void                        parse(std::string message);
        std::vector<std::string>    &getResult(void);
        std::string                 &getCommand();
        std::vector<std::string>    &getArguments();
        std::string                 &getMessage();

    private:
        std::string                 _command;
        std::vector<std::string>    _arguments;
        std::string                 _message;
        std::vector<std::string>    _result;
};

#endif
