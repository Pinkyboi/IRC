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
        std::vector<std::string>    &get_result(void);
        std::string                 &get_command();
        std::vector<std::string>    &get_arguments();
        std::string                 &get_message();
        bool                        &get_has_message();
        size_t                      &get_nargs();

    private:
        std::string                 _command;
        std::vector<std::string>    _arguments;
        std::string                 _message;
        std::vector<std::string>    _result;
        bool                        _has_message;
        size_t                      _nargs;
};

#endif
