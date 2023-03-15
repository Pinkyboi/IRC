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
        std::vector<std::string>    get_result(void) const;
        std::string                 get_command() const;
        std::vector<std::string>    get_arguments() const;
        std::string                 get_message() const;
        bool                        get_has_message() const;
        size_t                      get_nargs() const;

    private:
        std::string                 _command;
        std::vector<std::string>    _arguments;
        std::string                 _message;
        std::vector<std::string>    _result;
        bool                        _has_message;
        size_t                      _nargs;
};

#endif
