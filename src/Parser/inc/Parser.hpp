#ifndef PARSER_H
# define PARSER_H

# include <string>
# include <vector>
# include <iostream>
# include <bits/stdc++.h>
# include <regex>

class Parser
{
    public:
        Parser();
        ~Parser();
        void                     parse(std::string message);
        std::vector<std::string> &getResult(void);

    private:
        std::vector<std::string>    _result;
};

#endif