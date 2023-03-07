#ifndef COMMAND_BUFFER_H
# define COMMAND_BUFFER_H

#include <cstring>
#include <stdlib.h>

class CircularBuffer
{
    public:
                        CircularBuffer(size_t command_max_size);
                        CircularBuffer(size_t command_max_size, const char *str);
                        ~CircularBuffer();
    public:
        void            push_back(const char *str);
        void            clear();
    public:
        char            *get_buffer();
        size_t    get_size();
    private:
        char            *_buffer;
        size_t    _size;
        size_t    _capacity;
};

# endif