#include "CommandBuffer.hpp"

CircularBuffer::CircularBuffer(size_t command_max_size): _size(0), _capacity(command_max_size)
{
    _buffer = new char[command_max_size];
}

CircularBuffer::CircularBuffer(size_t command_max_size, const char *str): _size(0), _capacity(command_max_size)
{
    _buffer = new char[command_max_size];
    push_back(str);
}

CircularBuffer::~CircularBuffer()
{
    delete[] _buffer;
}

void    CircularBuffer::push_back(const char *str)
{
    for (int i = 0; str[i] ; i++)
    {
        _buffer[_size] = str[i];
        _size = (_size + 1) % _capacity;
    }
    _buffer[_size] = '\0';
}

void    CircularBuffer::clear()
{
    memset(_buffer, 0, _capacity);
    _size = 0;
}

char    *CircularBuffer::get_buffer()
{
    return (_buffer);
}

size_t  CircularBuffer::get_size()
{
    return (_size);
}