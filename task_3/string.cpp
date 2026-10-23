#include "./string.h"

#include <assert.h>
#include <stdio.h> 
#include <stdlib.h>

namespace
{
    char* malloc_data(size_t size)
    {
        char* result = new char[size + 1];
        if (result == nullptr)
        {
            fprintf(stderr, "Cannot allocate memroy\n");
            abort();
        }
    
        result[size] = '\0';
        return result;
    }

    // last - откуда ищем первое совпадение, 0 - слева, 1 - справа
    size_t search(const char* str, size_t str_len, const char* substr, size_t substr_len, bool last)
    {
        if (substr_len == 0)
            return last ? str_len : 0;
        
        if (substr_len > str_len)
            return -1;

        size_t result = -1;
        for (size_t i = 0; i <= str_len - substr_len; i++)
        {
            size_t j = 0;

            while (j < substr_len && str[i + j] == substr[j])
                j++;

            if (j == substr_len)
            {
                result = i;

                if (!last)
                    return result;
            }
        }

        return result;
    }
}

string::string() : data(malloc_data(0))
{
}

string::string(char val, size_t count) : data(malloc_data(count)), _size(count)
{
    for (size_t i = 0; i < _size; i++)
        data[i] = val;
}

string::string(const char* d)
{
    assert(d != nullptr);

    _size = 0;
    while (d[_size] != '\0')
        _size++;

    data = malloc_data(_size);
    for (size_t i = 0; i < _size; i++)
        data[i] = d[i];
}

string::string(const string& val) : data(malloc_data(val._size)), _size(val._size)
{
    for (size_t i = 0; i < _size; i++)
        data[i] = val.data[i];
}

string& string::operator=(const string& val)
{
    if (this == &val)
        return *this;

    char* new_data = malloc_data(val._size);
    for (size_t i = 0; i < val._size; i++)
        new_data[i] = val.data[i];

    delete[] data;
    data = new_data;
    _size = val._size;

    return *this;
}

string::string(string&& val) : string()
{
    *this = (string&&)val;
}

string& string::operator=(string&& val)
{
    if (this == &val)
        return *this;

    char* old_data = data;
    data = val.data;
    _size = val._size;
    val.data = old_data;
    val._size = 0;
    val.data[0] = '\0';

    return *this;
}

string string::operator+(const string& val)
{
    string result;
    size_t new_size = _size + val._size;
    char* new_data = malloc_data(new_size);

    for (size_t i = 0; i < _size; i++)
        new_data[i] = data[i];

    for (size_t i = 0; i < val._size; i++)
        new_data[_size + i] = val.data[i];

    delete[] result.data;
    result.data = new_data;
    result._size = new_size;

    return result;
}

string& string::operator+=(const string& val)
{
    if (val._size == 0)
        return *this;

    size_t new_size = _size + val._size;
    char* new_data = malloc_data(new_size);

    for (size_t i = 0; i < _size; i++)
        new_data[i] = data[i];

    for (size_t i = 0; i < val._size; i++)
        new_data[_size + i] = val.data[i];

    delete[] data;
    data = new_data;
    _size = new_size;
    
    return *this;
}

bool string::operator==(const string& val)
{
    if (_size != val._size)
        return 0;

    for (size_t i = 0; i < _size; i++)
        if (data[i] != val.data[i])
            return 0;

    return 1;
}

bool string::operator<(const string& val)
{
    size_t count = _size < val._size ? _size : val._size;

    for (size_t i = 0; i < count; i++)
    {
        unsigned char lhs = (unsigned char)data[i];
        unsigned char rhs = (unsigned char)val.data[i];
        
        if (lhs != rhs)
            return lhs < rhs;
    }

    return _size < val._size;
}

char string::operator[](size_t i) const
{
    assert(i <= _size);

    return data[i];
}

bool string::contains(const string& substr) const
{
    return (find(substr) != (size_t)-1);
}

// нужно заменять все подстроки what, а не только первую
bool string::replace(const string& what, const string& val)
{
    size_t count = 0;
    size_t pos = 0;

    while (pos <= _size)
    {
        size_t found = search(data + pos, _size - pos, what.data, what._size, 0);
        if (found == (size_t)-1)
            break;

        count++;
        pos += found + what._size;
        if (what._size == 0)
            pos++;
    }

    if (count == 0)
        return 0;

    size_t new_size = _size - count * what._size + count * val._size;
    char* new_data = malloc_data(new_size);
    size_t next = 0;
    pos = 0;

    for (size_t i = 0; i < count; i++)
    {
        size_t found = search(data + pos, _size - pos, what.data, what._size, 0);

        for (size_t j = 0; j < found; j++)
            new_data[next++] = data[pos + j];
        pos += found;

        for (size_t j = 0; j < val._size; j++)
            new_data[next++] = val.data[j];
        pos += what._size;

        if (what._size == 0 && pos < _size)
        {
            new_data[next++] = data[pos];
            pos++;
        }
    }

    for (size_t i = pos; i < _size; i++)
        new_data[next++] = data[i];

    delete[] data;
    data = new_data;
    _size = new_size;

    return 1;
}

bool string::empty() const
{
    return _size == 0;
}

string& string::append(const string& rhs)
{
    return (*this += rhs);
}

string& string::erase(size_t start, size_t end)
{
    assert(start <= end && end <= _size);

    size_t removed = end - start;
    if (removed == 0)
        return *this;

    for (size_t i = end; i < _size; i++)
        data[i - removed] = data[i];

    _size -= removed;
    data[_size] = '\0';

    return *this;
}

string string::substr(size_t start, size_t end) const
{
    assert(start <= end && end <= _size);

    string result('\0', end - start); // аоаоаоааао
    for (size_t i = start; i < end; i++)
        result.data[i - start] = data[i];
    
    return result;
}

size_t string::find(const string& substr) const
{
    return search(data, _size, substr.data, substr._size, 0);
}

size_t string::rfind(const string& substr) const
{
    return search(data, _size, substr.data, substr._size, 1);
}
